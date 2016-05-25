#include "configdialog.h"
#include "ui_configdialog.h"
#include "services/config.h"
#include "uiconfig.h"
#include "customconfigwidgetplugin.h"
#include "services/pluginmanager.h"
#include "formmanager.h"
#include "services/codeformatter.h"
#include "plugins/codeformatterplugin.h"
#include "configwidgets/styleconfigwidget.h"
#include "configwidgets/combodatawidget.h"
#include "configwidgets/listtostringlisthash.h"
#include "iconmanager.h"
#include "common/userinputfilter.h"
#include "multieditor/multieditorwidget.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "plugins/confignotifiableplugin.h"
#include "mainwindow.h"
#include "common/unused.h"
#include "sqlitestudio.h"
#include "configmapper.h"
#include "datatype.h"
#include "uiutils.h"
#include "translations.h"
#include <QSignalMapper>
#include <QLineEdit>
#include <QSpinBox>
#include <QDebug>
#include <QComboBox>
#include <QStyleFactory>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QDesktopServices>
#include <QtUiTools/QUiLoader>
#include <QKeySequenceEdit>
#include <plugins/uiconfiguredplugin.h>
#include <dbtree/dbtree.h>

#define GET_FILTER_STRING(Widget, WidgetType, Method) \
    if (qobject_cast<WidgetType*>(Widget))\
        return qobject_cast<WidgetType*>(Widget)->Method() + " " + Widget->toolTip();\

#define GET_FILTER_STRING2(Widget, WidgetType) \
    WidgetType* w##WidgetType = qobject_cast<WidgetType*>(widget);\
    if (w##WidgetType)\
        return getFilterString(w##WidgetType) + " " + Widget->toolTip();

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    init();
}

ConfigDialog::~ConfigDialog()
{
    // Cancel transaction on CfgMain objects from plugins
    rollbackPluginConfigs();

    // Notify plugins about dialog being closed
    UiConfiguredPlugin* cfgPlugin = nullptr;
    foreach (Plugin* plugin, PLUGINS->getLoadedPlugins())
    {
        cfgPlugin = dynamic_cast<UiConfiguredPlugin*>(plugin);
        if (!cfgPlugin)
            continue;

        cfgPlugin->configDialogClosed();
    }

    // Delete UI and other resources
    delete ui;
    safe_delete(configMapper);

    for (ConfigMapper* mapper : pluginConfigMappers.values())
        delete mapper;

    pluginConfigMappers.clear();

}

void ConfigDialog::configureDataEditors(const QString& dataTypeString)
{
    ui->categoriesWidget->setVisible(false);
    ui->stackedWidget->setCurrentWidget(ui->dataEditorsPage);

    for (int i = 0; i < ui->dataEditorsTypesList->count(); i++)
    {
        if (ui->dataEditorsTypesList->item(i)->text() == dataTypeString.toUpper())
        {
            ui->dataEditorsTypesList->setCurrentRow(i);
            return;
        }
    }

    addDataType(dataTypeString.toUpper());
}

QString ConfigDialog::getFilterString(QWidget *widget)
{
    // Common code for widgets with single method call
    GET_FILTER_STRING(widget, QLabel, text);
    GET_FILTER_STRING(widget, QAbstractButton, text);
    GET_FILTER_STRING(widget, QLineEdit, text);
    GET_FILTER_STRING(widget, QTextEdit, toPlainText);
    GET_FILTER_STRING(widget, QPlainTextEdit, toPlainText);
    GET_FILTER_STRING(widget, QGroupBox, title);
    GET_FILTER_STRING(widget, QKeySequenceEdit, keySequence().toString);

    // Widgets needs a little more than single method call
    GET_FILTER_STRING2(widget, QComboBox);
    GET_FILTER_STRING2(widget, QTreeWidget);
    GET_FILTER_STRING2(widget, QListWidget);
    GET_FILTER_STRING2(widget, QTableWidget);

    return QString::null;
}

QString ConfigDialog::getFilterString(QComboBox *widget)
{
    QStringList items;
    for (int i = 0; i < widget->count(); i++)
        items << widget->itemText(i);

    return items.join(" ");
}

QString ConfigDialog::getFilterString(QTreeWidget *widget)
{
    QList<QTreeWidgetItem*> items = widget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    QStringList strList;
    foreach (QTreeWidgetItem* item, items)
        for (int i = 0; i < widget->columnCount(); i++)
            strList << item->text(i) + " " + item->toolTip(0);

    return strList.join(" ");
}

QString ConfigDialog::getFilterString(QListWidget *widget)
{
    QList<QListWidgetItem*> items = widget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    QStringList strList;
    foreach (QListWidgetItem* item, items)
        strList << item->text() + " " + item->toolTip();

    return strList.join(" ");
}

QString ConfigDialog::getFilterString(QTableWidget *widget)
{
    QList<QTableWidgetItem*> items = widget->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
    QStringList strList;
    foreach (QTableWidgetItem* item, items)
         strList << item->text() + " " + item->toolTip();

    return strList.join(" ");
}

void ConfigDialog::init()
{
    ui->setupUi(this);
    setWindowIcon(ICONS.CONFIGURE);

    ui->categoriesTree->setCurrentItem(ui->categoriesTree->topLevelItem(0));

    configMapper = new ConfigMapper(CfgMain::getPersistableInstances());
    connectMapperSignals(configMapper);

    ui->categoriesFilter->setClearButtonEnabled(true);
    UserInputFilter* filter = new UserInputFilter(ui->categoriesFilter, this, SLOT(applyFilter(QString)));
    filter->setDelay(500);

    ui->stackedWidget->setCurrentWidget(ui->generalPage);
    initPageMap();
    initInternalCustomConfigWidgets();
    initPlugins();
    initPluginsPage();
    initFormatterPlugins();
    initDataEditors();
    initShortcuts();
    initLangs();

    connect(ui->categoriesTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(switchPage(QTreeWidgetItem*)));
    connect(ui->previewTabs, SIGNAL(currentChanged(int)), this, SLOT(updateStylePreview()));
    connect(ui->activeStyleCombo, SIGNAL(currentTextChanged(QString)), this, SLOT(updateStylePreview()));
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->hideBuiltInPluginsCheck, SIGNAL(toggled(bool)), this, SLOT(updateBuiltInPluginsVisibility()));

    QList<CfgEntry*> entries;
    entries << CFG_UI.General.SortObjects
            << CFG_UI.General.SortColumns
            << CFG_UI.General.ShowDbTreeLabels
            << CFG_UI.General.ShowRegularTableLabels
            << CFG_UI.General.ShowSystemObjects
            << CFG_UI.General.ShowVirtualTableLabels;

    for (CfgEntry* cfg : entries)
        connect(cfg, &CfgEntry::changed, this, &ConfigDialog::markRequiresSchemasRefresh);

    ui->activeStyleCombo->addItems(QStyleFactory::keys());

    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(pageSwitched()));

    ui->hideBuiltInPluginsCheck->setChecked(true);

#ifndef PORTABLE_CONFIG
    ui->updatesGroup->setVisible(false);
#endif

    load();
    updateStylePreview();
}

void ConfigDialog::load()
{
    updatingDataEditorItem = true;
    configMapper->loadToWidget(ui->stackedWidget);
    updatingDataEditorItem = false;
    setModified(false);
}

void ConfigDialog::save()
{
    if (MainWindow::getInstance()->currentStyle().compare(ui->activeStyleCombo->currentText(), Qt::CaseInsensitive) != 0)
        MainWindow::getInstance()->setStyle(ui->activeStyleCombo->currentText());

    QString loadedPlugins = collectLoadedPlugins();
    storeSelectedFormatters();
    CFG->beginMassSave();
    CFG_CORE.General.LoadedPlugins.set(loadedPlugins);
    configMapper->saveFromWidget(ui->stackedWidget, true);
    commitPluginConfigs();
    CFG->commitMassSave();

    if (requiresSchemasRefresh)
    {
        requiresSchemasRefresh = false;
        DBTREE->refreshSchemas();
    }
    MainWindow::getInstance()->updateCornerDocking();
}

void ConfigDialog::storeSelectedFormatters()
{
    CodeFormatterPlugin* plugin = nullptr;
    QTreeWidgetItem* item = nullptr;
    QComboBox* combo = nullptr;
    QString lang;
    QString pluginName;
    for (int i = 0, total = ui->formatterPluginsTree->topLevelItemCount(); i < total; ++i)
    {
        item = ui->formatterPluginsTree->topLevelItem(i);
        lang = item->text(0);

        combo = formatterLangToPluginComboMap[lang];
        if (!combo)
        {
            qCritical() << "Could not find combo for lang " << lang << " in storeSelectedFormatters()";
            continue;
        }

        pluginName = combo->currentData().toString();
        plugin = dynamic_cast<CodeFormatterPlugin*>(PLUGINS->getLoadedPlugin(pluginName));
        if (!plugin)
        {
            qCritical() << "Could not find plugin for lang " << lang << " in storeSelectedFormatters()";
            continue;
        }

        FORMATTER->setFormatter(lang, plugin);
    }

    FORMATTER->storeCurrentSettings();
}

void ConfigDialog::markModified()
{
    setModified(true);
}

void ConfigDialog::setModified(bool modified)
{
    modifiedFlag = modified;
    updateModified();
}

void ConfigDialog::updateModified()
{
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(modifiedFlag);
}

void ConfigDialog::applyFilter(const QString &filter)
{
    QColor normalColor = ui->categoriesTree->palette().color(QPalette::Active, QPalette::WindowText);
    QColor disabledColor = ui->categoriesTree->palette().color(QPalette::Disabled, QPalette::WindowText);
    if (filter.isEmpty())
    {
        foreach (QTreeWidgetItem* item, getAllCategoryItems())
            item->setForeground(0, normalColor);

        return;
    }

    QList<QWidget*> widgets = ui->stackedWidget->findChildren<QWidget*>();
    QList<QWidget*> matchedWidgets;
    foreach (QWidget* widget, widgets)
    {
        if (getFilterString(widget).contains(filter, Qt::CaseInsensitive))
            matchedWidgets << widget;
    }

    QHash<QWidget*, QTreeWidgetItem*> pageToCategoryItem = buildPageToCategoryItemMap();
    QSet<QTreeWidgetItem*> matchedCategories;
    foreach (QWidget* page, pageToCategoryItem.keys())
    {
        foreach (QWidget* matched, matchedWidgets)
        {
            if (page->isAncestorOf(matched))
            {
                if (!pageToCategoryItem.contains(page))
                {
                    qCritical() << "Page" << page << "not on page-to-category-item mapping.";
                    continue;
                }

                matchedCategories << pageToCategoryItem[page];
                break;
            }
        }
    }

    foreach (QTreeWidgetItem* item, getAllCategoryItems())
        item->setForeground(0, disabledColor);

    foreach (QTreeWidgetItem* item, matchedCategories)
    {
        item->setForeground(0, normalColor);
        while ((item = item->parent()) != nullptr)
            item->setForeground(0, normalColor);
    }
}

QHash<QWidget*, QTreeWidgetItem*> ConfigDialog::buildPageToCategoryItemMap() const
{
    QHash<QString,QTreeWidgetItem*> pageNameToCategoryItem;
    foreach (QTreeWidgetItem* item, getAllCategoryItems())
        pageNameToCategoryItem[item->statusTip(0)] = item;

    QWidget* page = nullptr;
    QHash<QWidget*,QTreeWidgetItem*> pageToCategoryItem;
    for (int i = 0; i < ui->stackedWidget->count(); i++)
    {
        page = ui->stackedWidget->widget(i);
        pageToCategoryItem[page] = pageNameToCategoryItem[page->objectName()];
    }
    return pageToCategoryItem;
}

QList<QTreeWidgetItem *> ConfigDialog::getAllCategoryItems() const
{
    return ui->categoriesTree->findItems("*", Qt::MatchWildcard|Qt::MatchRecursive);
}

QList<MultiEditorWidgetPlugin*> ConfigDialog::getDefaultEditorsForType(DataType::Enum dataType)
{
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();
    DataType modelDataType;
    modelDataType.setType(dataType);

    typedef QPair<int,MultiEditorWidgetPlugin*> PluginWithPriority;
    QList<PluginWithPriority> sortedPlugins;
    PluginWithPriority editorWithPrio;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        if (!plugin->validFor(modelDataType))
            continue;

        editorWithPrio.first = plugin->getPriority(modelDataType);
        editorWithPrio.second = plugin;
        sortedPlugins << editorWithPrio;
    }

    qSort(sortedPlugins.begin(), sortedPlugins.end(), [=](const PluginWithPriority& p1, const PluginWithPriority& p2) -> bool
    {
       return p1.first < p2.first;
    });

    QList<MultiEditorWidgetPlugin*> results;
    for (const PluginWithPriority& p: sortedPlugins)
        results << p.second;

    return results;
}

void ConfigDialog::pageSwitched()
{
    if (ui->stackedWidget->currentWidget() == ui->dataEditorsPage)
    {
        updateDataTypeEditors();
        return;
    }
}

void ConfigDialog::updateDataTypeEditors()
{
    QString typeName = ui->dataEditorsTypesList->currentItem()->text();
    DataType::Enum typeEnum = DataType::fromString(typeName);
    bool usingCustomOrder = false;
    QStringList editorsOrder = getPluginNamesFromDataTypeItem(ui->dataEditorsTypesList->currentItem(), &usingCustomOrder);
    QList<MultiEditorWidgetPlugin*> sortedPlugins;

    while (ui->dataEditorsSelectedTabs->count() > 0)
        delete ui->dataEditorsSelectedTabs->widget(0);

    ui->dataEditorsAvailableList->clear();
    if (usingCustomOrder)
        sortedPlugins = updateCustomDataTypeEditors(editorsOrder);
    else
        sortedPlugins = updateDefaultDataTypeEditors(typeEnum);

    ui->dataEditorsAvailableList->sortItems();

    for (MultiEditorWidgetPlugin* plugin : sortedPlugins)
        addDataTypeEditor(plugin);
}

QList<MultiEditorWidgetPlugin*> ConfigDialog::updateCustomDataTypeEditors(const QStringList& editorsOrder)
{
    // Building plugins list
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();
    QList<MultiEditorWidgetPlugin*> enabledPlugins;
    QListWidgetItem* item = nullptr;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        item = new QListWidgetItem(plugin->getTitle());
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setCheckState(editorsOrder.contains(plugin->getName()) ? Qt::Checked : Qt::Unchecked);
        item->setData(QListWidgetItem::UserType, plugin->getName());
        if (item->checkState() == Qt::Checked)
            enabledPlugins << plugin;

        ui->dataEditorsAvailableList->addItem(item);
    }

    qSort(enabledPlugins.begin(), enabledPlugins.end(), [=](MultiEditorWidgetPlugin* p1, MultiEditorWidgetPlugin* p2) -> bool
    {
        return editorsOrder.indexOf(p1->getName()) < editorsOrder.indexOf(p2->getName());
    });

    return enabledPlugins;
}

QList<MultiEditorWidgetPlugin*> ConfigDialog::updateDefaultDataTypeEditors(DataType::Enum typeEnum)
{
    // Building plugins list
    QList<MultiEditorWidgetPlugin*> plugins = PLUGINS->getLoadedPlugins<MultiEditorWidgetPlugin>();
    QList<MultiEditorWidgetPlugin*> enabledPlugins = getDefaultEditorsForType(typeEnum);
    QListWidgetItem* item = nullptr;
    for (MultiEditorWidgetPlugin* plugin : plugins)
    {
        item = new QListWidgetItem(plugin->getTitle());
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setCheckState(enabledPlugins.contains(plugin) ? Qt::Checked : Qt::Unchecked);
        item->setData(QListWidgetItem::UserType, plugin->getName());
        ui->dataEditorsAvailableList->addItem(item);
    }
    return enabledPlugins;
}

void ConfigDialog::addDataTypeEditor(const QString& pluginName)
{
    MultiEditorWidgetPlugin* plugin = dynamic_cast<MultiEditorWidgetPlugin*>(PLUGINS->getLoadedPlugin(pluginName));
    if (!plugin)
    {
        qCritical() << "Could not find plugin" << pluginName << " in ConfigDialog::addDataTypeEditor()";
        return;
    }

    addDataTypeEditor(plugin);
}

void ConfigDialog::addDataTypeEditor(MultiEditorWidgetPlugin* plugin)
{
    MultiEditorWidget* editor = plugin->getInstance();
    ui->dataEditorsSelectedTabs->addTab(editor, editor->getTabLabel().replace("&", "&&"));
}

void ConfigDialog::removeDataTypeEditor(QListWidgetItem* item, const QString& pluginName)
{
    QStringList orderedPlugins = getPluginNamesFromDataTypeItem(item);
    int idx = orderedPlugins.indexOf(pluginName);
    removeDataTypeEditor(idx);
}

void ConfigDialog::removeDataTypeEditor(int idx)
{
    if (idx < 0 || idx > (ui->dataEditorsSelectedTabs->count() - 1))
    {
        qCritical() << "Index out of range in ConfigDialog::removeDataTypeEditor():" << idx << "(tabs:" << ui->dataEditorsSelectedTabs->count() << ")";
        return;
    }

    delete ui->dataEditorsSelectedTabs->widget(idx);
}

void ConfigDialog::transformDataTypeEditorsToCustomList(QListWidgetItem* typeItem)
{
    DataType::Enum dataType = DataType::fromString(typeItem->text());
    QList<MultiEditorWidgetPlugin*> plugins = getDefaultEditorsForType(dataType);

    QStringList pluginNames;
    for (MultiEditorWidgetPlugin* plugin : plugins)
        pluginNames << plugin->getName();

    setPluginNamesForDataTypeItem(typeItem, pluginNames);
}

QStringList ConfigDialog::getPluginNamesFromDataTypeItem(QListWidgetItem* typeItem, bool* exists)
{
    QVariant data = typeItem->data(QListWidgetItem::UserType);
    if (exists)
        *exists = data.isValid();

    return data.toStringList();
}

void ConfigDialog::setPluginNamesForDataTypeItem(QListWidgetItem* typeItem, const QStringList& pluginNames)
{
    updatingDataEditorItem = true;
    typeItem->setData(QListWidgetItem::UserType, pluginNames);
    updatingDataEditorItem = false;
}

void ConfigDialog::addDataType(const QString& typeStr)
{
    QListWidgetItem* item = new QListWidgetItem(typeStr);
    item->setFlags(item->flags()|Qt::ItemIsEditable);
    ui->dataEditorsTypesList->addItem(item);
    ui->dataEditorsTypesList->setCurrentRow(ui->dataEditorsTypesList->count() - 1, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
    markModified();
}

void ConfigDialog::rollbackPluginConfigs()
{
    CfgMain* mainCfg = nullptr;
    for (UiConfiguredPlugin* plugin : pluginConfigMappers.keys())
    {
        mainCfg = plugin->getMainUiConfig();
        if (mainCfg)
            mainCfg->rollback();
    }
}

void ConfigDialog::commitPluginConfigs()
{
    CfgMain* mainCfg = nullptr;
    for (UiConfiguredPlugin* plugin : pluginConfigMappers.keys())
    {
        mainCfg = plugin->getMainUiConfig();
        if (mainCfg)
        {
            mainCfg->commit();
            mainCfg->begin(); // be prepared for further changes after "Apply"
        }
    }
}

void ConfigDialog::connectMapperSignals(ConfigMapper* mapper)
{
    connect(mapper, SIGNAL(modified()), this, SLOT(markModified()));
    connect(mapper, SIGNAL(notifyEnabledWidgetModified(QWidget*, CfgEntry*, const QVariant&)), this, SLOT(notifyPluginsAboutModification(QWidget*, CfgEntry*, const QVariant&)));
}

void ConfigDialog::updateDataTypeListState()
{
    bool listEditingEnabled = ui->dataEditorsTypesList->selectedItems().size() > 0 && ui->dataEditorsTypesList->currentItem()->flags().testFlag(Qt::ItemIsEditable);
    dataEditRenameAction->setEnabled(listEditingEnabled);
    dataEditDeleteAction->setEnabled(listEditingEnabled);

    bool orderEditingEnabled = ui->dataEditorsTypesList->selectedItems().size() > 0;
    ui->dataEditorsAvailableList->setEnabled(orderEditingEnabled);
    ui->dataEditorsSelectedTabs->setEnabled(orderEditingEnabled);
}

void ConfigDialog::dataEditorItemEdited(QListWidgetItem* item)
{
    if (updatingDataEditorItem)
        return;

    updatingDataEditorItem = true;
    QString txt = item->text().toUpper();
    if (DataType::getAllNames().contains(txt))
        txt += "_";

    while (ui->dataEditorsTypesList->findItems(txt, Qt::MatchExactly).size() > 1)
        txt += "_";

    item->setText(txt);
    updatingDataEditorItem = false;
}

void ConfigDialog::dataEditorAvailableChanged(QListWidgetItem* item)
{
    QListWidgetItem* typeItem = ui->dataEditorsTypesList->currentItem();
    if (!typeItem)
        return;

    bool exists = false;
    QStringList pluginNames = getPluginNamesFromDataTypeItem(typeItem, &exists);
    if (!exists)
    {
        transformDataTypeEditorsToCustomList(typeItem);
        pluginNames = getPluginNamesFromDataTypeItem(typeItem);
    }

    QString pluginName = item->data(QListWidgetItem::UserType).toString();
    Qt::CheckState state = item->checkState();
    if (pluginNames.contains(pluginName) && state == Qt::Unchecked)
    {
        removeDataTypeEditor(typeItem, pluginName);
        pluginNames.removeOne(pluginName);

    }
    else if (!pluginNames.contains(pluginName) && state == Qt::Checked)
    {
        addDataTypeEditor(pluginName);
        pluginNames << pluginName;
    }

    setPluginNamesForDataTypeItem(typeItem, pluginNames);
}

void ConfigDialog::dataEditorTabsOrderChanged(int from, int to)
{
    QListWidgetItem* typeItem = ui->dataEditorsTypesList->currentItem();
    if (!typeItem)
        return;

    bool exists = false;
    QStringList pluginNames = getPluginNamesFromDataTypeItem(typeItem, &exists);
    if (!exists)
    {
        transformDataTypeEditorsToCustomList(typeItem);
        pluginNames = getPluginNamesFromDataTypeItem(typeItem);
    }

    int pluginSize = pluginNames.size();
    if (from >= pluginSize || to >= pluginSize)
    {
        qCritical() << "Tabse moved out of range. in ConfigDialog::dataEditorTabsOrderChanged(). Range was: " << pluginSize << "and indexes were:" << from << to;
        return;
    }

    QString pluginName = pluginNames[from];
    pluginNames.removeAt(from);
    pluginNames.insert(to, pluginName);

    setPluginNamesForDataTypeItem(typeItem, pluginNames);
}

void ConfigDialog::addDataType()
{
    addDataType("");
    renameDataType();
}

void ConfigDialog::renameDataType()
{
    QListWidgetItem* item = ui->dataEditorsTypesList->currentItem();
    if (!item)
        return;

    ui->dataEditorsTypesList->editItem(item);
}

void ConfigDialog::delDataType()
{
    QListWidgetItem* item = ui->dataEditorsTypesList->currentItem();
    if (!item)
        return;

    int row = ui->dataEditorsTypesList->currentRow();
    delete ui->dataEditorsTypesList->takeItem(row);

    if (ui->dataEditorsTypesList->count() > 0)
    {
        if (ui->dataEditorsTypesList->count() <= row)
        {
            row--;
            if (row < 0)
                row = 0;
        }

        ui->dataEditorsTypesList->setCurrentRow(row, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
    }

    updateDataTypeListState();
    markModified();
}

void ConfigDialog::dataTypesHelp()
{
    static const QString url = QStringLiteral("http://wiki.sqlitestudio.pl/index.php/User_Manual#Customizing_data_type_editors");
    QDesktopServices::openUrl(QUrl(url, QUrl::StrictMode));
}

void ConfigDialog::updateActiveFormatterState()
{
    CodeFormatterPlugin* plugin = nullptr;
    QTreeWidgetItem* item = nullptr;
    QComboBox* combo = nullptr;
    QToolButton* button = nullptr;
    QString lang;
    QString pluginName;
    for (int i = 0, total = ui->formatterPluginsTree->topLevelItemCount(); i < total; ++i)
    {
        item = ui->formatterPluginsTree->topLevelItem(i);
        lang = item->text(0);

        combo = formatterLangToPluginComboMap[lang];
        button = formatterLangToConfigButtonMap[lang];
        if (!button)
        {
            qCritical() << "Could not find button for lang " << lang << " in updateActiveFormatterState()";
            continue;
        }

        if (!combo)
        {
            qCritical() << "Could not find combo for lang " << lang << " in updateActiveFormatterState()";
            button->setEnabled(false);
            continue;
        }

        pluginName = combo->currentData().toString();
        plugin = dynamic_cast<CodeFormatterPlugin*>(PLUGINS->getLoadedPlugin(pluginName));
        if (!plugin)
        {
            qCritical() << "Could not find plugin for lang " << lang << " in updateActiveFormatterState()";
            button->setEnabled(false);
            continue;
        }

        button->setEnabled(dynamic_cast<UiConfiguredPlugin*>(plugin));
    }
}

void ConfigDialog::configureFormatter(const QString& pluginTitle)
{
    QTreeWidgetItem* item = getItemByTitle(pluginTitle);
    if (!item)
        return;

    ui->categoriesTree->setCurrentItem(item);
}

void ConfigDialog::activeFormatterChanged()
{
    markModified();
    updateActiveFormatterState();
}

void ConfigDialog::detailsClicked(const QString& pluginName)
{
    static const QString details = QStringLiteral(
            "<table>"
                "<thead>"
                    "<tr><td colspan=2 align=\"center\"><b>%1</b></td></tr>"
                    "<tr><td colspan=2></td></tr>"
                "</thead>"
                "<tbody>%2</tbody>"
            "</table>");
    static const QString row = QStringLiteral("<tr><td>%1</td><td align=\"right\">%2</td></tr>");
    static const QString hline = QStringLiteral("<tr><td colspan=\"2\"><hr/></td></tr>");

    PluginType* type = PLUGINS->getPluginType(pluginName);
    Q_ASSERT(type != nullptr);

    // Rows
    QStringList rows;
    rows << row.arg(tr("Description:", "plugin details")).arg(PLUGINS->getDescription(pluginName));
    rows << row.arg(tr("Category:", "plugin details")).arg(type->getTitle());
    rows << row.arg(tr("Version:", "plugin details")).arg(PLUGINS->getPrintableVersion(pluginName));
    rows << row.arg(tr("Author:", "plugin details")).arg(PLUGINS->getAuthor(pluginName));
    rows << hline;
    rows << row.arg(tr("Internal name:", "plugin details")).arg(pluginName);
    rows << row.arg(tr("Dependencies:", "plugin details")).arg(PLUGINS->getDependencies(pluginName).join(", "));
    rows << row.arg(tr("Conflicts:", "plugin details")).arg(PLUGINS->getConflicts(pluginName).join(", "));

    // Message
    QString pluginDetails = details.arg(PLUGINS->getTitle(pluginName)).arg(rows.join(""));
    QMessageBox::information(this, tr("Plugin details"), pluginDetails);
}

void ConfigDialog::failedToLoadPlugin(const QString& pluginName)
{
    QTreeWidgetItem* theItem = itemToPluginNameMap.valueByRight(pluginName);
    if (!theItem)
    {
        qWarning() << "Plugin" << pluginName << "failed to load, but it could not be found on the plugins list in ConfigDialog.";
        return;
    }

    theItem->setCheckState(0, Qt::Unchecked);
}

void ConfigDialog::codeFormatterUnloaded()
{
    refreshFormattersPage();
}

void ConfigDialog::codeFormatterLoaded()
{
    refreshFormattersPage();
}

void ConfigDialog::loadUnloadPlugin(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    QString pluginName = itemToPluginNameMap.valueByLeft(item);
    if (PLUGINS->isBuiltIn(pluginName))
        return;

    bool wasLoaded = PLUGINS->isLoaded(pluginName);

    if (wasLoaded == (item->checkState(0) == Qt::Checked))
        return;

    if (wasLoaded)
        PLUGINS->unload(pluginName);
    else
        PLUGINS->load(pluginName);

    markModified();
}

void ConfigDialog::pluginAboutToUnload(Plugin* plugin, PluginType* type)
{
    // Deinit tree item
    QTreeWidgetItem* typeItem = getPluginsCategoryItem(type);
    QTreeWidgetItem* pluginItem = getPluginItem(plugin);
    if (pluginItem)
    {
        typeItem->removeChild(pluginItem);
        pluginToItemMap.remove(plugin);
    }

    // Notifiable plugin
    ConfigNotifiablePlugin* notifiablePlugin = dynamic_cast<ConfigNotifiablePlugin*>(plugin);
    if (notifiablePlugin && notifiablePlugins.contains(notifiablePlugin))
        notifiablePlugins.removeOne(notifiablePlugin);

    // Deinit page
    deinitPluginPage(plugin);

    // Update tree categories
    updatePluginCategoriesVisibility();
}

void ConfigDialog::pluginLoaded(Plugin* plugin, PluginType* type, bool skipConfigLoading)
{
    // Update formatters page
    if (type->isForPluginType<CodeFormatterPlugin>())
        codeFormatterLoaded();

    // Init page
    if (!initPluginPage(plugin, skipConfigLoading))
        return;

    // Init tree item
    QTreeWidgetItem* typeItem = getPluginsCategoryItem(type);
    QTreeWidgetItem* pluginItem = new QTreeWidgetItem({plugin->getTitle()});
    pluginItem->setStatusTip(0, plugin->getName());
    typeItem->addChild(pluginItem);
    pluginToItemMap[plugin] = pluginItem;

    // Update tree categories
    updatePluginCategoriesVisibility();

    // Notifiable plugin
    ConfigNotifiablePlugin* notifiablePlugin = dynamic_cast<ConfigNotifiablePlugin*>(plugin);
    if (notifiablePlugin)
        notifiablePlugins << notifiablePlugin;
}

void ConfigDialog::pluginUnloaded(const QString& pluginName, PluginType* type)
{
    UNUSED(pluginName);

    // Update formatters page
    if (type->isForPluginType<CodeFormatterPlugin>())
        codeFormatterUnloaded();
}

void ConfigDialog::updatePluginCategoriesVisibility()
{
    QTreeWidgetItem* categories = getPluginsCategoryItem();
    for (int i = 0; i < categories->childCount(); i++)
        updatePluginCategoriesVisibility(categories->child(i));
}

void ConfigDialog::updateBuiltInPluginsVisibility()
{
    bool hideBuiltIn = ui->hideBuiltInPluginsCheck->isChecked();
    QHashIterator<QTreeWidgetItem*,QString> it = itemToPluginNameMap.iterator();
    while (it.hasNext())
    {
        it.next();
        if (PLUGINS->isBuiltIn(it.value()))
            ui->pluginsList->setItemHidden(it.key(), hideBuiltIn);
        else
            ui->pluginsList->setItemHidden(it.key(), false);
    }
}

void ConfigDialog::applyShortcutsFilter(const QString &filter)
{
    QTreeWidgetItem* categoryItem = nullptr;
    QTreeWidgetItem* item = nullptr;
    QKeySequenceEdit* seqEdit = nullptr;
    bool empty = filter.isEmpty();
    bool visible = true;
    int foundInCategory = 0;
    for (int i = 0, total_i = ui->shortcutsTable->topLevelItemCount(); i < total_i; ++i)
    {
        foundInCategory = 0;
        categoryItem = ui->shortcutsTable->topLevelItem(i);
        for (int j = 0 , total_j = categoryItem->childCount(); j < total_j; ++j)
        {
            item = categoryItem->child(j);
            seqEdit = dynamic_cast<QKeySequenceEdit*>(ui->shortcutsTable->itemWidget(item, 1));
            visible = empty || item->text(0).contains(filter, Qt::CaseInsensitive) ||
                    seqEdit->keySequence().toString().contains(filter, Qt::CaseInsensitive);

            item->setHidden(!visible);
            if (visible)
                foundInCategory++;
        }

        categoryItem->setHidden(foundInCategory == 0);
    }
}

void ConfigDialog::markRequiresSchemasRefresh()
{
    requiresSchemasRefresh = true;
}

void ConfigDialog::notifyPluginsAboutModification(QWidget*, CfgEntry* key, const QVariant& value)
{
    for (ConfigNotifiablePlugin* plugin : notifiablePlugins)
        plugin->configModified(key, value);
}

void ConfigDialog::updatePluginCategoriesVisibility(QTreeWidgetItem* categoryItem)
{
    categoryItem->setHidden(categoryItem->childCount() == 0);
}

QString ConfigDialog::collectLoadedPlugins() const
{
    QStringList loaded;
    QHashIterator<QTreeWidgetItem*,QString> it = itemToPluginNameMap.iterator();
    while (it.hasNext())
    {
        it.next();
        loaded << (it.value() + "=" + ((it.key()->checkState(0) == Qt::Checked) ? "1" : "0"));
    }

    return loaded.join(",");
}

void ConfigDialog::initPageMap()
{
    int pages = ui->stackedWidget->count();
    QWidget* widget = nullptr;
    for (int i = 0; i < pages; i++)
    {
        widget = ui->stackedWidget->widget(i);
        nameToPage[widget->objectName()] = widget;
    }
}

void ConfigDialog::initInternalCustomConfigWidgets()
{
    QList<CustomConfigWidgetPlugin*> customWidgets;
    customWidgets << new StyleConfigWidget();
    customWidgets << new ListToStringListHash(&CFG_UI.General.DataEditorsOrder);
    customWidgets << new ComboDataWidget(&CFG_CORE.General.Language);
    configMapper->setInternalCustomConfigWidgets(customWidgets);
}

void ConfigDialog::initFormatterPlugins()
{
    ui->formatterPluginsTree->header()->setSectionsMovable(false);
    ui->formatterPluginsTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->formatterPluginsTree->resizeColumnToContents(1);
    ui->formatterPluginsTree->resizeColumnToContents(2);

    refreshFormattersPage();
}

void ConfigDialog::refreshFormattersPage()
{
    ui->formatterPluginsTree->clear();

    QHash<QString,QVariant> activeFormatters = CFG_CORE.General.ActiveCodeFormatter.get();

    QList<CodeFormatterPlugin*> plugins = PLUGINS->getLoadedPlugins<CodeFormatterPlugin>();
    QHash<QString,QList<CodeFormatterPlugin*>> groupedPlugins;
    for (CodeFormatterPlugin* plugin : plugins)
        groupedPlugins[plugin->getLanguage()] << plugin;

    formatterLangToPluginComboMap.clear();
    formatterLangToConfigButtonMap.clear();
    int row = 0;
    QTreeWidgetItem* item = nullptr;
    QComboBox* combo = nullptr;
    QToolButton* configButton = nullptr;
    QStringList pluginTitles;
    QStringList pluginNames;
    QStringList sortedPluginNames;
    QString selectedPluginName;
    QModelIndex index;
    QString groupName;
    QHashIterator<QString,QList<CodeFormatterPlugin*>> it(groupedPlugins);
    while (it.hasNext())
    {
        it.next();
        groupName = it.key();

        item = new QTreeWidgetItem({groupName});
        ui->formatterPluginsTree->addTopLevelItem(item);

        pluginNames.clear();
        pluginTitles.clear();
        for (CodeFormatterPlugin* plugin : it.value())
        {
            pluginNames << plugin->getName();
            pluginTitles << plugin->getTitle();
        }
        sortedPluginNames = pluginNames;
        qSort(sortedPluginNames);

        combo = new QComboBox(ui->formatterPluginsTree);
        for (int i = 0, total = pluginNames.size(); i < total; ++i)
            combo->addItem(pluginTitles[i], pluginNames[i]);

        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(activeFormatterChanged()));
        index = ui->formatterPluginsTree->model()->index(row, 1);
        ui->formatterPluginsTree->setIndexWidget(index, combo);
        formatterLangToPluginComboMap[groupName] = combo;

        if (activeFormatters.contains(groupName) && pluginNames.contains(activeFormatters[groupName].toString()))
        {
            selectedPluginName = activeFormatters[groupName].toString();
        }
        else
        {
            // Pick first from sorted list and put it to combobox
            selectedPluginName = sortedPluginNames.first();
        }

        configButton = new QToolButton(ui->formatterPluginsTree);
        configButton->setIcon(ICONS.CONFIGURE);
        index = ui->formatterPluginsTree->model()->index(row, 2);
        ui->formatterPluginsTree->setIndexWidget(index, configButton);
        connect(configButton, &QToolButton::clicked, [this, combo]() {configureFormatter(combo->currentText());});
        formatterLangToConfigButtonMap[groupName] = configButton;

        combo->setCurrentIndex(pluginNames.indexOf(selectedPluginName));

        row++;
    }

    updateActiveFormatterState();
}

void ConfigDialog::applyStyle(QWidget *widget, QStyle *style)
{
    widget->setStyle(style);
    foreach (QObject* child, widget->children())
    {
        if (!qobject_cast<QWidget*>(child))
            continue;

        applyStyle(qobject_cast<QWidget*>(child), style);
    }
}

QTreeWidgetItem* ConfigDialog::getPluginsCategoryItem() const
{
    QTreeWidgetItem* item = nullptr;
    for (int i = 0; i < ui->categoriesTree->topLevelItemCount(); i++)
    {
         item = ui->categoriesTree->topLevelItem(i);
         if (item->statusTip(0) == ui->pluginsPage->objectName())
             return item;
    }
    Q_ASSERT_X(true, "ConfigDialog", "No Plugins toplevel item in config categories tree!");
    return nullptr;
}

QTreeWidgetItem* ConfigDialog::getPluginsCategoryItem(PluginType* type) const
{
    if (!pluginTypeToItemMap.contains(type))
        return nullptr;

    return pluginTypeToItemMap[type];
}

QTreeWidgetItem* ConfigDialog::getPluginItem(Plugin* plugin) const
{
    if (!pluginToItemMap.contains(plugin))
        return nullptr;

    return pluginToItemMap[plugin];
}

QTreeWidgetItem* ConfigDialog::createPluginsTypeItem(const QString& widgetName, const QString& title) const
{
    if (FORMS->hasWidget(widgetName))
        return new QTreeWidgetItem({title});

    QTreeWidgetItem* pluginsCategoryItem = getPluginsCategoryItem();
    QTreeWidgetItem* item = nullptr;
    for (int i = 0; i < pluginsCategoryItem->childCount(); i++)
    {
        item = pluginsCategoryItem->child(i);
        if (item->statusTip(0) == widgetName)
            return item;
    }
    return nullptr;

}

QTreeWidgetItem* ConfigDialog::getItemByTitle(const QString& title) const
{
    QList<QTreeWidgetItem*> items = ui->categoriesTree->findItems(title, Qt::MatchExactly|Qt::MatchRecursive);
    if (items.size() == 0)
        return nullptr;

    return items.first();
}

void ConfigDialog::switchPage(QTreeWidgetItem *item)
{
    if (isPluginCategoryItem((item)))
    {
        switchPageToPlugin(item);
        return;
    }

    QString name = item->statusTip(0);
    if (!nameToPage.contains(name))
    {
        qWarning() << "Switched page to item" << name << "but there's no such named page defined in ConfigDialog.";
        return;
    }

    ui->stackedWidget->setCurrentWidget(nameToPage[name]);
}

void ConfigDialog::switchPageToPlugin(QTreeWidgetItem *item)
{
    QString pluginName = item->statusTip(0);
    if (!nameToPage.contains(pluginName))
    {
        qCritical() << "No plugin page available for plugin:" << pluginName;
        return;
    }
    ui->stackedWidget->setCurrentWidget(nameToPage[pluginName]);
}

void ConfigDialog::initPlugins()
{
    QTreeWidgetItem *item = getPluginsCategoryItem();

    // Recreate
    QTreeWidgetItem *typeItem = nullptr;
    foreach (PluginType* pluginType, PLUGINS->getPluginTypes())
    {
        typeItem = createPluginsTypeItem(pluginType->getConfigUiForm(), pluginType->getTitle());
        if (!typeItem)
            continue;

        item->addChild(typeItem);
        pluginTypeToItemMap[pluginType] = typeItem;

        foreach (Plugin* plugin, pluginType->getLoadedPlugins())
            pluginLoaded(plugin, pluginType, true);
    }

    updatePluginCategoriesVisibility();

    connect(PLUGINS, SIGNAL(loaded(Plugin*,PluginType*)), this, SLOT(pluginLoaded(Plugin*,PluginType*)));
    connect(PLUGINS, SIGNAL(aboutToUnload(Plugin*,PluginType*)), this, SLOT(pluginAboutToUnload(Plugin*,PluginType*)));
}

void ConfigDialog::initPluginsPage()
{
    setValidStateTooltip(ui->pluginsList, tr("Plugins are loaded/unloaded immediately when checked/unchecked, "
                                             "but modified list of plugins to load at startup is not saved until "
                                             "you commit the whole configuration dialog."));

    QTreeWidgetItem* category = nullptr;
    QTreeWidgetItem* item = nullptr;
    QFont font;
    QModelIndex categoryIndex;
    QModelIndex itemIndex;
    int itemRow;
    int categoryRow;
    bool builtIn;
    QLabel* detailsLabel = nullptr;
    QString title;
    QSize itemSize;
    QStringList pluginNames;

    // Font and metrics
    item = new QTreeWidgetItem({""});
    font = item->font(0);

    QFontMetrics fm(font);
    itemSize = QSize(-1, (fm.ascent() + fm.descent() + 4));

    delete item;

    // Creating...
    ui->pluginsList->header()->setSectionsMovable(false);
    ui->pluginsList->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    QBrush categoryBg = ui->pluginsList->palette().button();
    QBrush categoryFg = ui->pluginsList->palette().buttonText();

    connect(ui->pluginsList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(loadUnloadPlugin(QTreeWidgetItem*,int)));
    connect(PLUGINS, SIGNAL(failedToLoad(QString)), this, SLOT(failedToLoadPlugin(QString)));

    categoryRow = 0;
    QList<PluginType*> pluginTypes = PLUGINS->getPluginTypes();
    qSort(pluginTypes.begin(), pluginTypes.end(), PluginType::nameLessThan);
    foreach (PluginType* pluginType, pluginTypes)
    {
        category = new QTreeWidgetItem({pluginType->getTitle()});
        font.setItalic(false);
        font.setBold(true);
        category->setFont(0, font);
        for (int i = 0; i < 2; i++)
        {
            category->setBackground(i, categoryBg);
            category->setForeground(i, categoryFg);
        }
        category->setSizeHint(0, itemSize);
        ui->pluginsList->addTopLevelItem(category);

        categoryIndex = ui->pluginsList->model()->index(categoryRow, 0);
        categoryRow++;

        itemRow = 0;
        pluginNames = pluginType->getAllPluginNames();
        qSort(pluginNames);
        foreach (const QString& pluginName, pluginNames)
        {
            builtIn = PLUGINS->isBuiltIn(pluginName);
            title = PLUGINS->getTitle(pluginName);
            if (builtIn)
                title = tr("%1 (built-in)", "plugins manager in configuration dialog").arg(title);

            item = new QTreeWidgetItem({title});
            item->setCheckState(0, PLUGINS->isLoaded(pluginName) ? Qt::Checked : Qt::Unchecked);
            item->setSizeHint(0, itemSize);
            if (builtIn)
                item->setDisabled(true);

            category->addChild(item);

            itemToPluginNameMap.insert(item, pluginName);

            // Details button
            detailsLabel = new QLabel(QString("<a href='%1'>%2</a> ").arg(pluginName).arg(tr("Details")), ui->pluginsList);
            detailsLabel->setAlignment(Qt::AlignRight);
            itemIndex = ui->pluginsList->model()->index(itemRow, 1, categoryIndex);
            ui->pluginsList->setIndexWidget(itemIndex, detailsLabel);

            connect(detailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(detailsClicked(QString)));

            itemRow++;
        }

        if (itemRow == 0)
        {
            item = new QTreeWidgetItem({tr("No plugins in this category.")});
            item->setDisabled(true);
            item->setSizeHint(0, itemSize);

            font.setItalic(true);
            font.setBold(false);
            item->setFont(0, font);

            category->addChild(item);
        }

        category->setExpanded(true);
    }
}

bool ConfigDialog::initPluginPage(Plugin* plugin, bool skipConfigLoading)
{
    if (!dynamic_cast<UiConfiguredPlugin*>(plugin))
        return false;

    UiConfiguredPlugin* cfgPlugin = dynamic_cast<UiConfiguredPlugin*>(plugin);
    QString pluginName = plugin->getName();
    QString formName = cfgPlugin->getConfigUiForm();
    QWidget* widget = FORMS->createWidget(formName);
    if (!widget)
    {
        qWarning() << "Could not load plugin UI file" << formName << "for plugin:" << pluginName;
        return false;
    }

    nameToPage[pluginName] = widget;
    ui->stackedWidget->addWidget(widget);
    CfgMain* mainConfig = cfgPlugin->getMainUiConfig();
    if (mainConfig)
    {
        pluginConfigMappers[cfgPlugin] = new ConfigMapper(mainConfig);
        pluginConfigMappers[cfgPlugin]->bindToConfig(widget);
        connectMapperSignals(pluginConfigMappers[cfgPlugin]);
        mainConfig->begin();

        // Remove this config from global mapper (if present there), so it's handled per plugin mapper
        configMapper->removeMainCfgEntry(mainConfig);
        configMapper->ignoreWidget(widget);
    }
    else if (!skipConfigLoading)
    {
        configMapper->loadToWidget(widget);
    }

    cfgPlugin->configDialogOpen();
    return true;
}

void ConfigDialog::deinitPluginPage(Plugin* plugin)
{
    QString pluginName = plugin->getName();
    if (!nameToPage.contains(pluginName))
        return;

    if (dynamic_cast<UiConfiguredPlugin*>(plugin))
    {
        UiConfiguredPlugin* cfgPlugin = dynamic_cast<UiConfiguredPlugin*>(plugin);
        CfgMain* mainCfg = cfgPlugin->getMainUiConfig();
        if (mainCfg)
            mainCfg->rollback();

        cfgPlugin->configDialogClosed();

        if (pluginConfigMappers.contains(cfgPlugin))
        {
            delete pluginConfigMappers[cfgPlugin];
            pluginConfigMappers.remove(cfgPlugin);
        }
    }

    QWidget* widget = nameToPage[pluginName];
    nameToPage.remove(pluginName);
    configMapper->removeIgnoredWidget(widget);
    ui->stackedWidget->removeWidget(widget);
    delete widget;
}

void ConfigDialog::initDataEditors()
{
    ui->dataEditorsAvailableList->setSpacing(1);

    QHash<QString,QVariant> editorsOrder = CFG_UI.General.DataEditorsOrder.get();
    QSet<QString> dataTypeSet = editorsOrder.keys().toSet();
    dataTypeSet += DataType::getAllNames().toSet();
    QStringList dataTypeList = dataTypeSet.toList();
    qSort(dataTypeList);

    QListWidgetItem* item = nullptr;
    for (const QString& type : dataTypeList)
    {
        item = new QListWidgetItem(type);
        if (!DataType::getAllNames().contains(type))
            item->setFlags(item->flags()|Qt::ItemIsEditable);

        ui->dataEditorsTypesList->addItem(item);
    }

    QAction* act = new QAction(ICONS.INSERT_DATATYPE, tr("Add new data type"), ui->dataEditorsTypesToolbar);
    connect(act, SIGNAL(triggered()), this, SLOT(addDataType()));
    ui->dataEditorsTypesToolbar->addAction(act);

    dataEditRenameAction = new QAction(ICONS.RENAME_DATATYPE, tr("Rename selected data type"), ui->dataEditorsTypesToolbar);
    connect(dataEditRenameAction, SIGNAL(triggered()), this, SLOT(renameDataType()));
    ui->dataEditorsTypesToolbar->addAction(dataEditRenameAction);

    dataEditDeleteAction = new QAction(ICONS.DELETE_DATATYPE, tr("Delete selected data type"), ui->dataEditorsTypesToolbar);
    connect(dataEditDeleteAction, SIGNAL(triggered()), this, SLOT(delDataType()));
    ui->dataEditorsTypesToolbar->addAction(dataEditDeleteAction);

    act = new QAction(ICONS.HELP, tr("Help for configuring data type editors"), ui->dataEditorsTypesToolbar);
    connect(act, SIGNAL(triggered()), this, SLOT(dataTypesHelp()));
    ui->dataEditorsTypesToolbar->addAction(act);

    connect(ui->dataEditorsTypesList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateDataTypeEditors()));
    connect(ui->dataEditorsTypesList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateDataTypeListState()));
    connect(ui->dataEditorsTypesList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(dataEditorItemEdited(QListWidgetItem*)));
    connect(ui->dataEditorsAvailableList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(dataEditorAvailableChanged(QListWidgetItem*)));
    connect(ui->dataEditorsSelectedTabs->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(dataEditorTabsOrderChanged(int,int)));

    ui->dataEditorsTypesList->setCurrentRow(0, QItemSelectionModel::Clear|QItemSelectionModel::SelectCurrent);
    updateDataTypeListState();
}

void ConfigDialog::initShortcuts()
{
    ui->shortcutsTable->header()->setSectionsMovable(false);
    ui->shortcutsTable->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->shortcutsTable->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->shortcutsTable->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->shortcutsTable->header()->resizeSection(1, 150);
    ui->shortcutsTable->header()->resizeSection(2, 26);

    ui->shortcutsFilterEdit->setClearButtonEnabled(true);
    new UserInputFilter(ui->shortcutsFilterEdit, this, SLOT(applyShortcutsFilter(QString)));

    static const QString metaName = CFG_SHORTCUTS_METANAME;
    QList<CfgCategory*> categories;
    for (CfgMain* cfgMain : CfgMain::getInstances())
    {
        if (cfgMain->getMetaName() != metaName)
            continue;

        for (CfgCategory* cat : cfgMain->getCategories().values())
            categories << cat;
    }

    qSort(categories.begin(), categories.end(), [](CfgCategory* cat1, CfgCategory* cat2) -> bool
    {
        return cat1->getTitle().compare(cat2->getTitle()) < 0;
    });

    for (CfgCategory* cat : categories)
        initShortcuts(cat);
}

void ConfigDialog::initShortcuts(CfgCategory *cfgCategory)
{
    QTreeWidgetItem* item = nullptr;
    QFont font;
    QModelIndex categoryIndex;
    QModelIndex itemIndex;
    QKeySequenceEdit *sequenceEdit = nullptr;
    QToolButton* clearButton = nullptr;
    QString title;
    QSize itemSize;

    // Font and metrics
    item = new QTreeWidgetItem({""});
    font = item->font(0);

    QFontMetrics fm(font);
    itemSize = QSize(-1, (fm.ascent() + fm.descent() + 4));

    delete item;

    // Creating...
    QBrush categoryBg = ui->shortcutsTable->palette().button();
    QBrush categoryFg = ui->shortcutsTable->palette().buttonText();

    QTreeWidgetItem* category = new QTreeWidgetItem({cfgCategory->getTitle()});
    font.setItalic(false);
    font.setBold(true);
    category->setFont(0, font);
    for (int i = 0; i < 3; i++)
    {
        category->setBackground(i, categoryBg);
        category->setForeground(i, categoryFg);
    }
    category->setSizeHint(0, itemSize);
    category->setFlags(category->flags() ^ Qt::ItemIsSelectable);
    ui->shortcutsTable->addTopLevelItem(category);

    int categoryRow = ui->shortcutsTable->topLevelItemCount() - 1;
    categoryIndex = ui->shortcutsTable->model()->index(categoryRow, 0);

    int itemRow = 0;
    QStringList entryNames = cfgCategory->getEntries().keys();
    qSort(entryNames);
    foreach (const QString& entryName, entryNames)
    {
        // Title
        title = cfgCategory->getEntries()[entryName]->getTitle();
        item = new QTreeWidgetItem(category, {title});

        // Key edit
        sequenceEdit = new QKeySequenceEdit(ui->shortcutsTable);
        sequenceEdit->setFixedWidth(150);
        sequenceEdit->setProperty("cfg", cfgCategory->getEntries()[entryName]->getFullKey());
        itemIndex = ui->shortcutsTable->model()->index(itemRow, 1, categoryIndex);
        ui->shortcutsTable->setIndexWidget(itemIndex, sequenceEdit);
        configMapper->addExtraWidget(sequenceEdit);

        // Clear button
        clearButton = new QToolButton(ui->shortcutsTable);
        clearButton->setIcon(ICONS.CLEAR_LINEEDIT);
        connect(clearButton, &QToolButton::clicked, [this, sequenceEdit]()
        {
            sequenceEdit->clear();
            this->markModified();

        });
        itemIndex = ui->shortcutsTable->model()->index(itemRow, 2, categoryIndex);
        ui->shortcutsTable->setIndexWidget(itemIndex, clearButton);

        itemRow++;
    }

    category->setExpanded(true);
}

void ConfigDialog::initLangs()
{
    QMap<QString, QString> langs = getAvailableLanguages();
    int idx = 0;
    int selected = -1;
    for (const QString& lang : langs.keys())
    {
        ui->langCombo->addItem(lang, langs[lang]);
        if (langs[lang] == SQLITESTUDIO->getCurrentLang())
            selected = idx;

        idx++;
    }

    ui->langCombo->setCurrentIndex(selected);
}

bool ConfigDialog::isPluginCategoryItem(QTreeWidgetItem *item) const
{
    return item->parent() && item->parent()->parent() && item->parent()->parent() == getPluginsCategoryItem();
}

void ConfigDialog::updateStylePreview()
{
    ui->previewWidget->parentWidget()->layout()->removeWidget(ui->previewWidget);
    ui->previewTabs->currentWidget()->layout()->addWidget(ui->previewWidget);
    ui->previewWidget->setEnabled(ui->previewTabs->currentIndex() == 0);

    QStyle* previousStyle = previewStyle;
    previewStyle = QStyleFactory::create(ui->activeStyleCombo->currentText());
    if (!previewStyle)
    {
        qWarning() << "Could not create style:" << ui->activeStyleCombo->currentText();
        return;
    }

    applyStyle(ui->activeStylePreviewGroup, previewStyle);

    if (previousStyle)
        delete previousStyle;
}

void ConfigDialog::apply()
{
    if (modifiedFlag)
        save();

    setModified(false);
}

void ConfigDialog::accept()
{
    apply();
    QDialog::accept();
}
