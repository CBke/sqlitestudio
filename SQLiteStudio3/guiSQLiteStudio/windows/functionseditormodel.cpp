#include "functionseditormodel.h"
#include "common/strhash.h"
#include "common/unused.h"
#include "services/pluginmanager.h"
#include "plugins/scriptingplugin.h"
#include "icon.h"
#include <QDebug>

#define SETTER(X, Y) \
    if (!isValidRowIndex(row) || X == Y) \
        return; \
    \
    X = Y; \
    emitDataChanged(row);

#define GETTER(X, Y) \
    if (!isValidRowIndex(row)) \
        return Y; \
    \
    return X;

FunctionsEditorModel::FunctionsEditorModel(QObject *parent) :
    QAbstractListModel(parent)
{
    init();
}

void FunctionsEditorModel::clearModified()
{
    beginResetModel();
    foreach (Function* func, functionList)
        func->modified = false;

    listModified = false;
    originalFunctionList = functionList;

    endResetModel();
}

bool FunctionsEditorModel::isModified() const
{
    if (functionList != originalFunctionList)
        return true;

    foreach (Function* func, functionList)
    {
        if (func->modified)
            return true;
    }
    return false;
}

bool FunctionsEditorModel::isModified(int row) const
{
    GETTER(functionList[row]->modified, false);
}

void FunctionsEditorModel::setModified(int row, bool modified)
{
    SETTER(functionList[row]->modified, modified);
}

bool FunctionsEditorModel::isValid() const
{
    foreach (Function* func, functionList)
    {
        if (!func->valid)
            return false;
    }
    return true;
}

bool FunctionsEditorModel::isValid(int row) const
{
    GETTER(functionList[row]->valid, false);
}

void FunctionsEditorModel::setValid(int row, bool valid)
{
    SETTER(functionList[row]->valid, valid);
}

void FunctionsEditorModel::setCode(int row, const QString& code)
{
    SETTER(functionList[row]->data.code, code);
}

QString FunctionsEditorModel::getCode(int row) const
{
    GETTER(functionList[row]->data.code, QString::null);
}

void FunctionsEditorModel::setFinalCode(int row, const QString& code)
{
    SETTER(functionList[row]->data.finalCode, code);
}

QString FunctionsEditorModel::getFinalCode(int row) const
{
    GETTER(functionList[row]->data.finalCode, QString::null);
}

void FunctionsEditorModel::setInitCode(int row, const QString& code)
{
    SETTER(functionList[row]->data.initCode, code);
}

QString FunctionsEditorModel::getInitCode(int row) const
{
    GETTER(functionList[row]->data.initCode, QString::null);
}

void FunctionsEditorModel::setName(int row, const QString& newName)
{
    SETTER(functionList[row]->data.name, newName);
}

QString FunctionsEditorModel::getName(int row) const
{
    GETTER(functionList[row]->data.name, QString::null);
}

void FunctionsEditorModel::setLang(int row, const QString& lang)
{
    SETTER(functionList[row]->data.lang, lang);
}

QString FunctionsEditorModel::getLang(int row) const
{
    GETTER(functionList[row]->data.lang, QString::null);
}

bool FunctionsEditorModel::getUndefinedArgs(int row) const
{
    GETTER(functionList[row]->data.undefinedArgs, true);
}

void FunctionsEditorModel::setUndefinedArgs(int row, bool value)
{
    SETTER(functionList[row]->data.undefinedArgs, value);
}

bool FunctionsEditorModel::getAllDatabases(int row) const
{
    GETTER(functionList[row]->data.allDatabases, true);
}

void FunctionsEditorModel::setAllDatabases(int row, bool value)
{
    SETTER(functionList[row]->data.allDatabases, value);
}

FunctionManager::ScriptFunction::Type FunctionsEditorModel::getType(int row) const
{
    GETTER(functionList[row]->data.type, FunctionManager::ScriptFunction::SCALAR);
}

void FunctionsEditorModel::setType(int row, FunctionManager::ScriptFunction::Type type)
{
    SETTER(functionList[row]->data.type, type);
}

bool FunctionsEditorModel::isAggregate(int row) const
{
    GETTER(functionList[row]->data.type == FunctionManager::ScriptFunction::AGGREGATE, false);
}

bool FunctionsEditorModel::isScalar(int row) const
{
    GETTER(functionList[row]->data.type == FunctionManager::ScriptFunction::SCALAR, false);
}

QStringList FunctionsEditorModel::getArguments(int row) const
{
    GETTER(functionList[row]->data.arguments, QStringList());
}

void FunctionsEditorModel::setArguments(int row, const QStringList& value)
{
    SETTER(functionList[row]->data.arguments, value);
}

QStringList FunctionsEditorModel::getDatabases(int row) const
{
    GETTER(functionList[row]->data.databases, QStringList());
}

void FunctionsEditorModel::setDatabases(int row, const QStringList& value)
{
    SETTER(functionList[row]->data.databases, value);
}

void FunctionsEditorModel::setData(const QList<FunctionManager::ScriptFunction*>& functions)
{
    beginResetModel();

    for (Function* functionPtr : functionList)
        delete functionPtr;

    functionList.clear();

    foreach (FunctionManager::ScriptFunction* func, functions)
        functionList << new Function(func);

    listModified = false;
    originalFunctionList = functionList;

    endResetModel();
}

void FunctionsEditorModel::addFunction(FunctionManager::ScriptFunction* function)
{
    int row = functionList.size();

    beginInsertRows(QModelIndex(), row, row);

    functionList << new Function(function);
    listModified = true;

    endInsertRows();
}

void FunctionsEditorModel::deleteFunction(int row)
{
    if (!isValidRowIndex(row))
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete functionList[row];
    functionList.removeAt(row);

    listModified = true;

    endRemoveRows();
}

QList<FunctionManager::ScriptFunction*> FunctionsEditorModel::generateFunctions() const
{
    QList<FunctionManager::ScriptFunction*> results;

    foreach (Function* func, functionList)
        results << new FunctionManager::ScriptFunction(func->data);

    return results;
}

QStringList FunctionsEditorModel::getFunctionNames() const
{
    QStringList names;
    foreach (Function* func, functionList)
        names << func->data.name;

    return names;
}

void FunctionsEditorModel::validateNames()
{
    StrHash<QList<int>> counter;

    int row = 0;
    foreach (Function* func, functionList)
    {
        func->valid &= true;
        counter[func->data.name] << row++;
    }

    QHashIterator<QString,QList<int>> cntIt = counter.iterator();
    while (cntIt.hasNext())
    {
        cntIt.next();
        if (cntIt.value().size() > 1)
        {
            foreach (int cntRow, cntIt.value())
                setValid(cntRow, false);
        }
    }

    QModelIndex idx;
    for (int i = 0; i < functionList.size(); i++)
    {
        idx = index(i);
        emit dataChanged(idx, idx);
    }
}

bool FunctionsEditorModel::isAllowedName(int rowToSkip, const QString& nameToValidate)
{
    QStringList names = getFunctionNames();
    names.removeAt(rowToSkip);
    return !names.contains(nameToValidate, Qt::CaseInsensitive);
}

int FunctionsEditorModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    return functionList.size();
}

QVariant FunctionsEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRowIndex(index.row()))
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        Function* fn = functionList[index.row()];
        return fn->data.toString();
    }

    if (role == Qt::DecorationRole && langToIcon.contains(functionList[index.row()]->data.lang))
    {
        QIcon icon = langToIcon[functionList[index.row()]->data.lang];
        if (!functionList[index.row()]->valid)
            icon = Icon::merge(icon, Icon::ERROR);

        return icon;
    }

    return QVariant();
}

void FunctionsEditorModel::init()
{
    foreach (ScriptingPlugin* plugin, PLUGINS->getLoadedPlugins<ScriptingPlugin>())
        langToIcon[plugin->getLanguage()] = QIcon(plugin->getIconPath());
}

bool FunctionsEditorModel::isValidRowIndex(int row) const
{
    return (row >= 0 && row < functionList.size());
}

void FunctionsEditorModel::emitDataChanged(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

FunctionsEditorModel::Function::Function()
{
}

FunctionsEditorModel::Function::Function(FunctionManager::ScriptFunction* other)
{
    data = FunctionManager::ScriptFunction(*other);
    originalName = data.name;
}
