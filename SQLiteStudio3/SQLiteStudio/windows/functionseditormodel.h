#ifndef FUNCTIONSEDITORMODEL_H
#define FUNCTIONSEDITORMODEL_H

#include "services/config.h"
#include "services/functionmanager.h"
#include <QIcon>
#include <QAbstractListModel>

class FunctionsEditorModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        enum Role
        {
            CODE = 1000,
            MODIFIED = 1001,
            VALID = 1002,
            TYPE = 1003
        };

        explicit FunctionsEditorModel(QObject *parent = 0);

        void clearModified();
        bool isModified() const;
        bool isModified(int row) const;
        void setModified(int row, bool modified);
        bool isValid(int row) const;
        void setValid(int row, bool valid);
        void setCode(int row, const QString& code);
        QString getCode(int row) const;
        void setFinalCode(int row, const QString& code);
        QString getFinalCode(int row) const;
        void setInitCode(int row, const QString& code);
        QString getInitCode(int row) const;
        void setName(int row, const QString& newName);
        QString getName(int row) const;
        void setLang(int row, const QString& lang);
        QString getLang(int row) const;
        QStringList getDatabases(int row) const;
        void setDatabases(int row, const QStringList& value);
        QStringList getArguments(int row) const;
        void setArguments(int row, const QStringList& value);
        FunctionManager::Function::Type getType(int row) const;
        void setType(int row, FunctionManager::Function::Type type);
        bool isAggregate(int row) const;
        bool isScalar(int row) const;
        bool getUndefinedArgs(int row) const;
        void setUndefinedArgs(int row, bool value);
        bool getAllDatabases(int row) const;
        void setAllDatabases(int row, bool value);
        void setData(const QList<FunctionManager::FunctionPtr>& functions);
        void addFunction(const FunctionManager::FunctionPtr& function);
        void deleteFunction(int row);
        QList<FunctionManager::FunctionPtr> getFunctions() const;
        QStringList getFunctionNames() const;
        void validateNames();
        bool isAllowedName(int rowToSkip, const QString& nameToValidate);
        bool isValidRow(int row) const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index, int role) const;

    private:
        struct Function
        {
            Function();
            Function(const FunctionManager::FunctionPtr& other);

            FunctionManager::FunctionPtr data;
            bool modified = false;
            bool valid = true;
            QString originalName;
        };

        void init();
        void emitDataChanged(int row);

        QList<Function*> functionList;

        /**
         * @brief List of function pointers before modifications.
         *
         * This list is kept to check for modifications in the overall list of functions.
         * Pointers on this list may be already deleted, so don't use them!
         * It's only used to compare list of pointers to functionList, so it can tell you
         * if the list was modified in regards of adding or deleting functions.
         */
        QList<Function*> originalFunctionList;
        QHash<QString,QIcon> langToIcon;
        bool listModified = false;
};

#endif // FUNCTIONSEDITORMODEL_H
