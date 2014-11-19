#ifndef COLLATIONSEDITOR_H
#define COLLATIONSEDITOR_H

#include "mdichild.h"
#include "common/extactioncontainer.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QWidget>

namespace Ui {
    class CollationsEditor;
}

class SyntaxHighlighterPlugin;
class SelectableDbModel;
class CollationsEditorModel;
class QSortFilterProxyModel;
class QSyntaxHighlighter;

class GUI_API_EXPORT CollationsEditor : public MdiChild
{
        Q_OBJECT

    public:
        enum Action
        {
            COMMIT,
            ROLLBACK,
            ADD,
            DELETE,
            HELP
        };

        enum ToolBar
        {
            TOOLBAR
        };

        explicit CollationsEditor(QWidget *parent = 0);
        ~CollationsEditor();

        bool restoreSessionNextTime();
        bool isUncommited() const;
        QString getQuitUncommitedConfirmMessage() const;

    protected:
        QVariant saveSession();
        bool restoreSession(const QVariant &sessionValue);
        Icon* getIconNameForMdiWindow();
        QString getTitleForMdiWindow();
        void createActions();
        void setupDefShortcuts();
        QToolBar* getToolBar(int toolbar) const;

    private:
        void init();
        int getCurrentCollationRow() const;
        void collationDeselected(int row);
        void collationSelected(int row);
        void clearEdits();
        void selectCollation(int row);
        QStringList getCurrentDatabases() const;
        void setFont(const QFont& font);

        Ui::CollationsEditor *ui = nullptr;
        CollationsEditorModel* model = nullptr;
        QSortFilterProxyModel* collationFilterModel = nullptr;
        SelectableDbModel* dbListModel = nullptr;
        QHash<QString,SyntaxHighlighterPlugin*> highlighterPlugins;
        QSyntaxHighlighter* currentHighlighter = nullptr;
        QString currentHighlighterLang;
        bool currentModified = false;
        bool updatesForSelection = false;

    private slots:
        void help();
        void commit();
        void rollback();
        void newCollation();
        void deleteCollation();
        void updateState();
        void updateCurrentCollationState();
        void collationSelected(const QItemSelection& selected, const QItemSelection& deselected);
        void updateModified();
        void applyFilter(const QString& value);
        void changeFont(const QVariant& font);
};

#endif // COLLATIONSEDITOR_H
