#ifndef SQLITECREATEINDEX_H
#define SQLITECREATEINDEX_H

#include "sqlitequery.h"
#include "sqlitetablerelatedddl.h"
#include "sqliteconflictalgo.h"
#include "sqliteexpr.h"
#include "sqliteddlwithdbcontext.h"
#include <QString>
#include <QList>

class SqliteIndexedColumn;

class API_EXPORT SqliteCreateIndex : public SqliteQuery, public SqliteTableRelatedDdl, public SqliteDdlWithDbContext
{
    public:
        SqliteCreateIndex();
        SqliteCreateIndex(const SqliteCreateIndex& other);
        SqliteCreateIndex(bool unique, bool ifNotExists, const QString& name1, const QString& name2,
                          const QString& name3, const QList<SqliteIndexedColumn*>& columns,
                          SqliteConflictAlgo onConflict = SqliteConflictAlgo::null);
        SqliteCreateIndex(bool unique, bool ifNotExists, const QString& name1, const QString& name2,
                          const QString& name3, const QList<SqliteIndexedColumn*>& columns,
                          SqliteExpr* where);
        ~SqliteCreateIndex();
        SqliteStatement* clone();

        QString getTargetTable() const;
        QString getTargetDatabase() const;
        void setTargetDatabase(const QString& database);

        bool uniqueKw = false;
        bool ifNotExistsKw = false;
        QList<SqliteIndexedColumn*> indexedColumns;
        // The database refers to index name in Sqlite3, but in Sqlite2 it refers to the table.
        QString database = QString::null;
        QString index = QString::null;
        QString table = QString::null;
        SqliteConflictAlgo onConflict = SqliteConflictAlgo::null;
        SqliteExpr* where = nullptr;

    protected:
        QStringList getTablesInStatement();
        QStringList getDatabasesInStatement();
        TokenList getTableTokensInStatement();
        TokenList getDatabaseTokensInStatement();
        QList<FullObject> getFullObjectsInStatement();
        TokenList rebuildTokensFromContents();
};

typedef QSharedPointer<SqliteCreateIndex> SqliteCreateIndexPtr;

#endif // SQLITECREATEINDEX_H
