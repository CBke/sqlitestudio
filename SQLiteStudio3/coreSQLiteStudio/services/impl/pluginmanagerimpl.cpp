#include "pluginmanagerimpl.h"
#include "plugins/scriptingplugin.h"
#include "plugins/genericplugin.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

PluginManagerImpl::PluginManagerImpl()
{
}

PluginManagerImpl::~PluginManagerImpl()
{
}

void PluginManagerImpl::init()
{
    pluginDirs += qApp->applicationDirPath() + "/plugins";
    pluginDirs += QDir(CFG->getConfigDir()).absoluteFilePath("plugins");

    QString envDirs = SQLITESTUDIO->getEnv("SQLITESTUDIO_PLUGINS");
    if (!envDirs.isNull())
        pluginDirs += envDirs.split(PATH_LIST_SEPARATOR);

#ifdef PLUGINS_DIR
    pluginDirs += PLUGINS_DIR;
#endif

#ifdef SYS_PLUGINS_DIR
    pluginDirs += SYS_PLUGINS_DIR;
#endif

    scanPlugins();
    loadPlugins();
}

void PluginManagerImpl::deinit()
{
    emit aboutToQuit();

    // Plugin containers and their plugins
    foreach (PluginContainer* container, pluginContainer.values())
    {
        if (container->builtIn)
        {
            container->plugin->deinit();
            delete container->plugin;
        }
        else
            unload(container->name);
    }

    foreach (PluginContainer* container, pluginContainer.values())
        delete container;

    pluginContainer.clear();

    // Types
    foreach (PluginType* type, registeredPluginTypes)
        delete type;

    registeredPluginTypes.clear();
    pluginCategories.clear();
}

QList<PluginType*> PluginManagerImpl::getPluginTypes() const
{
    return registeredPluginTypes;
}

QStringList PluginManagerImpl::getPluginDirs() const
{
    return pluginDirs;
}

QString PluginManagerImpl::getFilePath(Plugin* plugin) const
{
    if (!pluginContainer.contains(plugin->getName()))
        return QString::null;

    return pluginContainer[plugin->getName()]->filePath;
}

bool PluginManagerImpl::loadBuiltInPlugin(Plugin* plugin)
{
    bool res = initPlugin(plugin);
    res &= plugin->init();
    return res;
}

PluginType* PluginManagerImpl::getPluginType(Plugin* plugin) const
{
    if (!pluginContainer.contains(plugin->getName()))
        return nullptr;

    return pluginContainer[plugin->getName()]->type;
}

void PluginManagerImpl::scanPlugins()
{
    QStringList nameFilters;
    nameFilters << "*.so" << "*.dll";

    QPluginLoader* loader;
    foreach (QString pluginDirPath, pluginDirs)
    {
        QDir pluginDir(pluginDirPath);
        foreach (QString fileName, pluginDir.entryList(nameFilters, QDir::Files))
        {
            fileName = pluginDir.absoluteFilePath(fileName);
            loader = new QPluginLoader(fileName);
            loader->setLoadHints(QLibrary::ExportExternalSymbolsHint|QLibrary::ResolveAllSymbolsHint);

            if (!initPlugin(loader, fileName))
            {
                qDebug() << "File" << fileName << "was loaded as plugin, but SQLiteStudio couldn't initialize plugin.";
                delete loader;
            }
        }
    }

    QStringList names;
    for (PluginContainer* container : pluginContainer.values())
    {
        if (!container->builtIn)
            names << container->name;
    }

    qDebug() << "Following plugins found:" << names;
}

void PluginManagerImpl::loadPlugins()
{
    QStringList alreadyAttempted;
    for (const QString& pluginName : pluginContainer.keys())
    {
        if (shouldAutoLoad(pluginName))
            load(pluginName, alreadyAttempted);
    }

    pluginsAreInitiallyLoaded = true;
    emit pluginsInitiallyLoaded();
}

bool PluginManagerImpl::initPlugin(QPluginLoader* loader, const QString& fileName)
{
    QJsonObject pluginMetaData = loader->metaData();
    QString pluginTypeName = pluginMetaData.value("MetaData").toObject().value("type").toString();
    PluginType* pluginType = nullptr;
    foreach (PluginType* type, registeredPluginTypes)
    {
        if (type->getName() == pluginTypeName)
        {
            pluginType = type;
            break;
        }
    }

    if (!pluginType)
    {
        qWarning() << "Could not load plugin" + fileName + "because its type was not recognized:" << pluginTypeName;
        return false;
    }

    QString pluginName = pluginMetaData.value("className").toString();
    QJsonObject metaObject = pluginMetaData.value("MetaData").toObject();

    if (!checkPluginRequirements(pluginName, metaObject))
        return false;

    PluginContainer* container = new PluginContainer;
    container->type = pluginType;
    container->filePath = fileName;
    container->loaded = false;
    container->loader = loader;
    pluginCategories[pluginType] << container;
    pluginContainer[pluginName] = container;

    if (!readDependencies(pluginName, container, metaObject.value("dependencies")))
        return false;

    if (!readConflicts(pluginName, container, metaObject.value("conflicts")))
        return false;

    if (!readMetaData(container))
    {
        delete container;
        return false;
    }

    return true;
}

bool PluginManagerImpl::checkPluginRequirements(const QString& pluginName, const QJsonObject& metaObject)
{
    if (metaObject.value("gui").toBool(false) && !SQLITESTUDIO->isGuiAvailable())
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires GUI and this is not GUI client running.";
        return false;
    }

    int minVer = metaObject.value("minQtVersion").toInt(0);
    if (QT_VERSION_CHECK(minVer / 10000, minVer / 100 % 100, minVer % 10000) > QT_VERSION)
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires at least Qt version" << toPrintableVersion(minVer) << ", but got" << QT_VERSION_STR;
        return false;
    }

    int maxVer = metaObject.value("maxQtVersion").toInt(999999);
    if (QT_VERSION_CHECK(maxVer / 10000, maxVer / 100 % 100, maxVer % 10000) < QT_VERSION)
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires at most Qt version" << toPrintableVersion(maxVer) << ", but got" << QT_VERSION_STR;
        return false;
    }

    minVer = metaObject.value("minAppVersion").toInt(0);
    if (SQLITESTUDIO->getVersion() < minVer)
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires at least SQLiteStudio version" << toPrintableVersion(minVer) << ", but got"
                 << SQLITESTUDIO->getVersionString();
        return false;
    }

    maxVer = metaObject.value("maxAppVersion").toInt(999999);
    if (SQLITESTUDIO->getVersion() > maxVer)
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires at most SQLiteStudio version" << toPrintableVersion(maxVer) << ", but got"
                 << SQLITESTUDIO->getVersionString();
        return false;
    }

    return true;
}

bool PluginManagerImpl::readDependencies(const QString& pluginName, PluginManagerImpl::PluginContainer* container, const QJsonValue& depsValue)
{
    if (depsValue.isUndefined())
        return true;

    QJsonArray depsArray;
    if (depsValue.type() == QJsonValue::Array)
        depsArray = depsValue.toArray();
    else
        depsArray.append(depsValue);

    PluginDependency dep;
    QJsonObject depObject;
    for (const QJsonValue& value : depsArray)
    {
        if (value.type() == QJsonValue::Object)
        {
            depObject = value.toObject();
            if (!depObject.contains("name"))
            {
                qWarning() << "Invalid dependency entry in plugin" << pluginName << " - doesn't contain 'name' of the dependency.";
                return false;
            }

            dep.name = depObject.value("name").toString();
            dep.minVersion = depObject.value("minVersion").toInt(0);
            dep.maxVersion = depObject.value("maxVersion").toInt(0);
        }
        else
        {
            dep.maxVersion = 0;
            dep.minVersion = 0;
            dep.name = value.toString();
        }
        container->dependencies << dep;
    }
    return true;
}

bool PluginManagerImpl::readConflicts(const QString& pluginName, PluginManagerImpl::PluginContainer* container, const QJsonValue& confValue)
{
    UNUSED(pluginName);

    if (confValue.isUndefined())
        return true;

    QJsonArray confArray;
    if (confValue.type() == QJsonValue::Array)
        confArray = confValue.toArray();
    else
        confArray.append(confValue);

    for (const QJsonValue& value : confArray)
        container->conflicts << value.toString();

    return true;
}

bool PluginManagerImpl::initPlugin(Plugin* plugin)
{
    QString pluginName = plugin->getName();
    PluginType* pluginType = nullptr;
    foreach (PluginType* type, registeredPluginTypes)
    {
        if (type->test(plugin))
        {
            pluginType = type;
            break;
        }
    }

    if (!pluginType)
    {
        qWarning() << "Could not load built-in plugin" + pluginName + "because its type was not recognized.";
        return false;
    }

    PluginContainer* container = new PluginContainer;
    container->type = pluginType;
    container->loaded = true;
    container->builtIn = true;
    container->plugin = plugin;
    pluginCategories[pluginType] << container;
    pluginContainer[pluginName] = container;
    if (!readMetaData(container))
    {
        delete container;
        return false;
    }

    pluginLoaded(container);
    return true;
}

bool PluginManagerImpl::shouldAutoLoad(const QString& pluginName)
{
    QStringList loadedPlugins = CFG_CORE.General.LoadedPlugins.get().split(",", QString::SkipEmptyParts);
    QStringList pair;
    foreach (const QString& loadedPlugin, loadedPlugins)
    {
        pair = loadedPlugin.split("=");
        if (pair.size() != 2)
        {
            qWarning() << "Invalid entry in config General.LoadedPlugins:" << loadedPlugin;
            continue;
        }

        if (pair[0] == pluginName)
            return (bool)pair[1].toInt();
    }

    return true;
}

QStringList PluginManagerImpl::getAllPluginNames(PluginType* type) const
{
    QStringList names;
    if (!pluginCategories.contains(type))
        return names;

    foreach (PluginContainer* container, pluginCategories[type])
        names << container->name;

    return names;
}

QStringList PluginManagerImpl::getAllPluginNames() const
{
    return pluginContainer.keys();
}

PluginType* PluginManagerImpl::getPluginType(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return nullptr;

    return pluginContainer[pluginName]->type;
}

QString PluginManagerImpl::getAuthor(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->author;
}

QString PluginManagerImpl::getTitle(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->title;
}

QString PluginManagerImpl::getPrintableVersion(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->printableVersion;
}

int PluginManagerImpl::getVersion(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return 0;

    return pluginContainer[pluginName]->version;
}

QString PluginManagerImpl::getDescription(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->description;
}

void PluginManagerImpl::unload(Plugin* plugin)
{
    if (!plugin)
        return;

    unload(plugin->getName());
}

void PluginManagerImpl::unload(const QString& pluginName)
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to unload plugin.";
        return;
    }

    // Checking preconditions
    PluginContainer* container = pluginContainer[pluginName];
    if (container->builtIn)
        return;

    if (!container->loaded)
        return;

    // Unloading depdendent plugins
    for (PluginContainer* otherContainer : pluginContainer.values())
    {
        if (otherContainer == container)
            continue;

        for (const PluginDependency& dep : otherContainer->dependencies)
        {
            if (dep.name == pluginName)
            {
                unload(otherContainer->name);
                break;
            }
        }
    }

    // Removing from fast-lookup collections
    removePluginFromCollections(container->plugin);

    // Deinitializing and unloading plugin
    emit aboutToUnload(container->plugin, container->type);
    container->plugin->deinit();

    QPluginLoader* loader = container->loader;
    if (!loader->isLoaded())
    {
        qWarning() << "QPluginLoader says the plugin is not loaded. Weird.";
        emit unloaded(container->name, container->type);
        return;
    }

    loader->unload();

    container->plugin = nullptr;
    container->loaded = false;

    emit unloaded(container->name, container->type);

    qDebug() << pluginName << "unloaded:" << container->filePath;
}

bool PluginManagerImpl::load(const QString& pluginName)
{
    QStringList alreadyAttempted;
    bool res = load(pluginName, alreadyAttempted);
    if (!res)
        emit failedToLoad(pluginName);

    return res;
}

bool PluginManagerImpl::load(const QString& pluginName, QStringList& alreadyAttempted, int minVersion, int maxVersion)
{
    if (alreadyAttempted.contains(pluginName))
        return false;

    // Checking initial conditions
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to load plugin.";
        alreadyAttempted.append(pluginName);
        return false;
    }

    PluginContainer* container = pluginContainer[pluginName];

    if (minVersion > 0 && container->version < minVersion)
    {
        qWarning() << "Requested plugin" << pluginName << "in version at least" << minVersion << "but have:" << container->version;
        return false;
    }

    if (maxVersion > 0 && container->version > maxVersion)
    {
        qWarning() << "Requested plugin" << pluginName << "in version at most" << maxVersion << "but have:" << container->version;
        return false;
    }

    if (container->builtIn)
        return true;

    QPluginLoader* loader = container->loader;
    if (loader->isLoaded())
        return true;

    // Checking for conflicting plugins
    for (PluginContainer* otherContainer : pluginContainer.values())
    {
        if (!otherContainer->loaded || otherContainer->name == pluginName)
            continue;

        if (container->conflicts.contains(otherContainer->name) || otherContainer->conflicts.contains(pluginName))
        {
            notifyWarn(tr("Cannot load plugin %1, because it's in conflict with plugin %2.").arg(pluginName, otherContainer->name));
            alreadyAttempted.append(pluginName);
            return false;
        }
    }

    // Loading depended plugins
    for (const PluginDependency& dep : container->dependencies)
    {
        if (!load(dep.name, alreadyAttempted, dep.minVersion, dep.maxVersion))
        {
            notifyWarn(tr("Cannot load plugin %1, because its dependency was not loaded: %2.").arg(pluginName, dep.name));
            alreadyAttempted.append(pluginName);
            return false;
        }
    }

    // Loading pluginName
    if (!loader->load())
    {
        notifyWarn(tr("Cannot load plugin %1. Error details: %2").arg(pluginName, loader->errorString()));
        alreadyAttempted.append(pluginName);
        return false;
    }

    // Initializing loaded plugin
    Plugin* plugin = dynamic_cast<Plugin*>(container->loader->instance());
    GenericPlugin* genericPlugin = dynamic_cast<GenericPlugin*>(plugin);
    if (genericPlugin)
    {
        genericPlugin->loadMetaData(container->loader->metaData());
    }

    if (!plugin->init())
    {
        loader->unload();
        notifyWarn(tr("Cannot load plugin %1 (error while initializing plugin).").arg(pluginName));
        alreadyAttempted.append(pluginName);
        return false;
    }

    pluginLoaded(container);

    return true;
}

void PluginManagerImpl::pluginLoaded(PluginManagerImpl::PluginContainer* container)
{
    if (!container->builtIn)
    {
        container->plugin = dynamic_cast<Plugin*>(container->loader->instance());
        container->loaded = true;
    }
    addPluginToCollections(container->plugin);

    emit loaded(container->plugin, container->type);
    if (!container->builtIn)
        qDebug() << container->name << "loaded:" << container->filePath;
}

void PluginManagerImpl::addPluginToCollections(Plugin* plugin)
{
    ScriptingPlugin* scriptingPlugin = dynamic_cast<ScriptingPlugin*>(plugin);
    if (scriptingPlugin)
        scriptingPlugins[scriptingPlugin->getLanguage()] = scriptingPlugin;
}

void PluginManagerImpl::removePluginFromCollections(Plugin* plugin)
{
    ScriptingPlugin* scriptingPlugin = dynamic_cast<ScriptingPlugin*>(plugin);
    if (scriptingPlugin && scriptingPlugins.contains(scriptingPlugin->getLanguage()))
        scriptingPlugins.remove(plugin->getName());
}

bool PluginManagerImpl::readMetaData(PluginManagerImpl::PluginContainer* container)
{
    if (container->loader)
    {
        QHash<QString, QVariant> metaData = readMetaData(container->loader->metaData());
        container->name = metaData["name"].toString();
        container->version = metaData["version"].toInt();
        container->printableVersion = toPrintableVersion(metaData["version"].toInt());
        container->author = metaData["author"].toString();
        container->description = metaData["description"].toString();
        container->title = metaData["title"].toString();
    }
    else if (container->plugin)
    {
        container->name = container->plugin->getName();
        container->version = container->plugin->getVersion();
        container->printableVersion = container->plugin->getPrintableVersion();
        container->author = container->plugin->getAuthor();
        container->description = container->plugin->getDescription();
        container->title = container->plugin->getTitle();
    }
    else
    {
        qCritical() << "Could not read metadata for some plugin. It has no loader or plugin object defined.";
        return false;
    }
    return true;
}

bool PluginManagerImpl::isLoaded(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to get plugin 'loaded' status.";
        return false;
    }

    return pluginContainer[pluginName]->loaded;
}

bool PluginManagerImpl::isBuiltIn(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to get plugin 'builtIn' status.";
        return false;
    }

    return pluginContainer[pluginName]->builtIn;
}

Plugin* PluginManagerImpl::getLoadedPlugin(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return nullptr;

    if (!pluginContainer[pluginName]->loaded)
        return nullptr;

    return pluginContainer[pluginName]->plugin;
}

QList<Plugin*> PluginManagerImpl::getLoadedPlugins(PluginType* type) const
{
    QList<Plugin*> list;
    if (!pluginCategories.contains(type))
        return list;

    foreach (PluginContainer* container, pluginCategories[type])
    {
        if (container->loaded)
            list << container->plugin;
    }

    return list;
}

ScriptingPlugin* PluginManagerImpl::getScriptingPlugin(const QString& languageName) const
{
    if (scriptingPlugins.contains(languageName))
        return scriptingPlugins[languageName];

    return nullptr;
}

QHash<QString, QVariant> PluginManagerImpl::readMetaData(const QJsonObject& metaData)
{
    QHash<QString, QVariant> results;
    results["name"] = metaData.value("className").toString();

    QJsonObject root = metaData.value("MetaData").toObject();
    results["type"] = root.value("type").toString();
    results["title"] = root.value("title").toString();
    results["description"] = root.value("description").toString();
    results["author"] = root.value("author").toString();
    results["version"] = root.value("version").toInt();
    results["ui"] = root.value("ui").toString();
    return results;
}

QString PluginManagerImpl::toPrintableVersion(int version) const
{
    static const QString versionStr = QStringLiteral("%1.%2.%3");
    return versionStr.arg(version / 10000)
                     .arg(version / 100 % 100)
                     .arg(version % 100);
}

QStringList PluginManagerImpl::getDependencies(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QStringList();

    static const QString verTpl = QStringLiteral(" (%1)");
    QString minVerTpl = tr("min: %1", "plugin dependency version");
    QString maxVerTpl = tr("max: %1", "plugin dependency version");
    QStringList outputList;
    QString depStr;
    QStringList depVerList;
    for (const PluginDependency& dep : pluginContainer[pluginName]->dependencies)
    {
        depStr = dep.name;
        if (dep.minVersion > 0 || dep.maxVersion > 0)
        {
            depVerList.clear();
            if (dep.minVersion > 0)
                depVerList << minVerTpl.arg(toPrintableVersion(dep.minVersion));

            if (dep.maxVersion > 0)
                depVerList << minVerTpl.arg(toPrintableVersion(dep.maxVersion));

            depStr += verTpl.arg(depVerList.join(", "));
        }
        outputList << depStr;
    }

    return outputList;
}

QStringList PluginManagerImpl::getConflicts(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QStringList();

    return pluginContainer[pluginName]->conflicts;
}

bool PluginManagerImpl::arePluginsInitiallyLoaded() const
{
    return pluginsAreInitiallyLoaded;
}

QList<Plugin*> PluginManagerImpl::getLoadedPlugins() const
{
    QList<Plugin*> plugins;
    foreach (PluginContainer* container, pluginContainer.values())
    {
        if (container->loaded)
            plugins << container->plugin;
    }
    return plugins;
}

QStringList PluginManagerImpl::getLoadedPluginNames() const
{
    QStringList names;
    foreach (PluginContainer* container, pluginContainer.values())
    {
        if (container->loaded)
            names << container->name;
    }
    return names;
}

void PluginManagerImpl::registerPluginType(PluginType* type)
{
    registeredPluginTypes << type;
}
