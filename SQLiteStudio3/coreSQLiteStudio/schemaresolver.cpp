#include "schemaresolver.h"
#include "db/db.h"
#include "db/sqlresultsrow.h"
#include "parser/parsererror.h"
#include "parser/ast/sqlitecreatetable.h"
#include "parser/ast/sqlitecreateindex.h"
#include "parser/ast/sqlitecreatetrigger.h"
#include "parser/ast/sqlitecreateview.h"
#include "parser/ast/sqlitecreatevirtualtable.h"
#include "parser/ast/sqlitetablerelatedddl.h"
#include <QDebug>

const char* sqliteMasterDdl =
    "CREATE TABLE sqlite_master (type text, name text, tbl_name text, rootpage integer, sql text)";
const char* sqliteTempMasterDdl =
    "CREATE TABLE sqlite_temp_master (type text, name text, tbl_name text, rootpage integer, sql text)";

ExpiringCache<SchemaResolver::ObjectCacheKey,QVariant> SchemaResolver::cache;

SchemaResolver::SchemaResolver(Db *db)
    : db(db)
{
    parser = new Parser(db->getDialect());
}

SchemaResolver::~SchemaResolver()
{
    delete parser;
}

QStringList SchemaResolver::getTables(const QString &database)
{
    QStringList tables = getObjects(database, "table");
    if (!ignoreSystemObjects)
        tables << "sqlite_master" << "sqlite_temp_master";

    return tables;
}

QStringList SchemaResolver::getIndexes(const QString &database)
{
    QStringList indexes = getObjects(database, "index");
    if (ignoreSystemObjects)
        filterSystemIndexes(indexes);

    return indexes;
}

QStringList SchemaResolver::getTriggers(const QString &database)
{
    return getObjects(database, "trigger");
}

QStringList SchemaResolver::getViews(const QString &database)
{
    return getObjects(database, "view");
}

StrHash<QStringList> SchemaResolver::getGroupedIndexes(const QString &database)
{
    QStringList allIndexes = getIndexes(database);
    return getGroupedObjects(database, allIndexes, SqliteQueryType::CreateIndex);
}

StrHash<QStringList> SchemaResolver::getGroupedTriggers(const QString &database)
{
    QStringList allTriggers = getTriggers(database);
    return getGroupedObjects(database, allTriggers, SqliteQueryType::CreateTrigger);
}

StrHash< QStringList> SchemaResolver::getGroupedObjects(const QString &database, const QStringList &inputList, SqliteQueryType type)
{
    QString strType = sqliteQueryTypeToString(type);
    StrHash< QStringList> groupedTriggers;

    SqliteQueryPtr parsedQuery;
    SqliteTableRelatedDdlPtr tableRelatedDdl;

    foreach (QString object, inputList)
    {
        parsedQuery = getParsedObject(database, object, ANY);
        if (!parsedQuery)
        {
            qWarning() << "Could not get parsed object for " << strType << ":" << object;
            continue;
        }

        tableRelatedDdl  = parsedQuery.dynamicCast<SqliteTableRelatedDdl>();
        if (!tableRelatedDdl)
        {
            qWarning() << "Parsed object is not of expected type. Expected" << strType
                       << ", but got" << sqliteQueryTypeToString(parsedQuery->queryType);
            continue;
        }

        groupedTriggers[tableRelatedDdl->getTargetTable()] << object;
    }

    return groupedTriggers;
}

bool SchemaResolver::isFilteredOut(const QString& value, const QString& type)
{
    if (ignoreSystemObjects)
    {
        if (type == "table" && isSystemTable(value))
            return true;

        if (type == "index" && isSystemIndex(value, db->getDialect()))
            return true;
    }

    return false;
}

QSet<QString> SchemaResolver::getDatabases()
{
    return db->getAllAttaches();
}

QStringList SchemaResolver::getTableColumns(const QString& table)
{
    return getTableColumns("main", table);
}

QStringList SchemaResolver::getTableColumns(const QString &database, const QString &table)
{
    QStringList columns; // result

    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return columns;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    SqliteCreateVirtualTablePtr createVirtualTable = query.dynamicCast<SqliteCreateVirtualTable>();
    if (!createTable && !createVirtualTable)
    {
        qDebug() << "Parsed DDL is neither a CREATE TABLE or CREATE VIRTUAL TABLE statement. It's: "
                 << sqliteQueryTypeToString(query->queryType);

        return columns;
    }

    // If we parsed virtual table, then we have to create temporary regular table to extract columns.
    if (createVirtualTable)
    {
        createTable = virtualTableAsRegularTable(database, table);
        if (!createTable)
            return columns;
    }

    // Now we have a regular table, let's extract columns.
    foreach (SqliteCreateTable::Column* column, createTable->columns)
        columns << column->name;

    return columns;
}

QList<DataType> SchemaResolver::getTableColumnDataTypes(const QString& table, int expectedNumberOfTypes)
{
    return getTableColumnDataTypes("main", table, expectedNumberOfTypes);
}

QList<DataType> SchemaResolver::getTableColumnDataTypes(const QString& database, const QString& table, int expectedNumberOfTypes)
{
    QList<DataType> dataTypes;
    SqliteCreateTablePtr createTable = getParsedObject(database, table, TABLE).dynamicCast<SqliteCreateTable>();
    if (!createTable)
    {
        for (int i = 0; i < expectedNumberOfTypes; i++)
            dataTypes << DataType();

        return dataTypes;
    }

    for (SqliteCreateTable::Column* col : createTable->columns)
    {
        if (!col->type)
        {
            dataTypes << DataType();
            continue;
        }

        dataTypes << col->type->toDataType();
    }

    for (int i = dataTypes.size(); i < expectedNumberOfTypes; i++)
        dataTypes << DataType();

    return dataTypes;
}

StrHash<QStringList> SchemaResolver::getAllTableColumns(const QString &database)
{
    StrHash< QStringList> tableColumns;
    foreach (QString table, getTables(database))
        tableColumns[table] = getTableColumns(database, table);

    return tableColumns;
}

QStringList SchemaResolver::getViewColumns(const QString& view)
{
    return getViewColumns("main", view);
}

QStringList SchemaResolver::getViewColumns(const QString& database, const QString& view)
{
    QList<SelectResolver::Column> resolvedColumns = getViewColumnObjects(database, view);
    QStringList columns;
    foreach (const SelectResolver::Column& col, resolvedColumns)
        columns << col.displayName;

    return columns;
}

QList<SelectResolver::Column> SchemaResolver::getViewColumnObjects(const QString& view)
{
    return getViewColumnObjects("main", view);
}

QList<SelectResolver::Column> SchemaResolver::getViewColumnObjects(const QString& database, const QString& view)
{
    QList<SelectResolver::Column> results;
    SqliteQueryPtr query = getParsedObject(database, view, VIEW);
    if (!query)
        return results;

    SqliteCreateViewPtr createView = query.dynamicCast<SqliteCreateView>();
    if (!createView)
    {
        qDebug() << "Parsed query is not CREATE VIEW statement as expected.";
        return results;
    }

    SelectResolver resolver(db, createView->select->detokenize());
    QList<QList<SelectResolver::Column> > resolvedColumns = resolver.resolve(createView->select);
    if (resolvedColumns.size() == 0)
    {
        qDebug() << "Could not resolve any results column from the view object.";
        return results;
    }
    return resolvedColumns.first();
}

SqliteCreateTablePtr SchemaResolver::virtualTableAsRegularTable(const QString &database, const QString &table)
{
    Dialect dialect = db->getDialect();
    QString strippedName = stripObjName(table, dialect);
    QString dbName = getPrefixDb(database, dialect);

    // Create temp table to see columns.
    QString newTable = db->getUniqueNewObjectName(strippedName);
    QString origTable = wrapObjIfNeeded(strippedName, dialect);
    SqlQueryPtr tempTableRes = db->exec(QString("CREATE TEMP TABLE %1 AS SELECT * FROM %2.%3 LIMIT 0;").arg(newTable, dbName, origTable), dbFlags);
    if (tempTableRes->isError())
        qWarning() << "Could not create temp table to identify virtual table columns of virtual table " << origTable << ". Error details:" << tempTableRes->getErrorText();

    // Get parsed DDL of the temp table.
    SqliteQueryPtr query = getParsedObject("temp", newTable, TABLE);
    if (!query)
        return SqliteCreateTablePtr();

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();

    // Getting rid of the temp table.
    db->exec(QString("DROP TABLE %1;").arg(newTable), dbFlags);

    // Returning results. Might be null.
    return createTable;
}

QString SchemaResolver::getObjectDdl(const QString& name, ObjectType type)
{
    return getObjectDdl("main", name, type);
}

QString SchemaResolver::getObjectDdl(const QString &database, const QString &name, ObjectType type)
{
    if (name.isNull())
        return QString::null;

    Dialect dialect = db->getDialect();
    // In case of sqlite_master or sqlite_temp_master we have static definitions
    QString lowerName = stripObjName(name, dialect).toLower();
    if (lowerName == "sqlite_master")
        return getSqliteMasterDdl(false);
    else if (lowerName == "sqlite_temp_master")
        return getSqliteMasterDdl(true);

    // Prepare db prefix.
    QString dbName = getPrefixDb(database, dialect);

    // Standalone or temp table?
    QString targetTable = "sqlite_master";
    if (database.toLower() == "temp")
        targetTable = "sqlite_temp_master";

    // Cache
    QString typeStr = objectTypeToString(type);
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_DDL, db, dbName, lowerName, typeStr);
    if (useCache && cache.contains(key))
        return cache.object(key, true)->toString();

    // Get the DDL
    QString resStr = getObjectDdlWithSimpleName(dbName, lowerName, targetTable, type);
    if (resStr.isNull())
        resStr = getObjectDdlWithDifficultName(dbName, lowerName, targetTable, type);

    // If the DDL doesn't have semicolon at the end (usually the case), add it.
    if (!resStr.trimmed().endsWith(";"))
        resStr += ";";

    if (useCache)
        cache.insert(key, new QVariant(resStr));

    // Return the DDL
    return resStr;
}

QString SchemaResolver::getObjectDdlWithDifficultName(const QString &dbName, const QString &lowerName, QString targetTable, SchemaResolver::ObjectType type)
{
    //
    // Slower, but works with Russian names, etc, because "string lower" is done only at Qt level, not at SQLite level.
    //
    QString typeStr = objectTypeToString(type);
    SqlQueryPtr queryResults;
    if (type != ANY)
    {
        queryResults = db->exec(QString(
                    "SELECT name, sql FROM %1.%4 WHERE type = '%3';").arg(dbName, typeStr, targetTable),
                    dbFlags
                );

    }
    else
    {
        queryResults = db->exec(QString(
                    "SELECT name, sql FROM %1.%3;").arg(dbName, targetTable),
                    dbFlags
                );
    }

    // Validate query results
    if (queryResults->isError())
    {
        qDebug() << "Could not get object's DDL:" << dbName << "." << lowerName << ", details:" << queryResults->getErrorText();
        return QString::null;
    }

    // The DDL string
    SqlResultsRowPtr row;
    while (queryResults->hasNext())
    {
        row = queryResults->next();
        if (row->value("name").toString().toLower() != lowerName)
            continue;

        return row->value("sql").toString();
    }
    return QString();
}

QString SchemaResolver::getObjectDdlWithSimpleName(const QString &dbName, const QString &lowerName, QString targetTable, SchemaResolver::ObjectType type)
{
    QString typeStr = objectTypeToString(type);
    QVariant results;
    SqlQueryPtr queryResults;
    if (type != ANY)
    {
        queryResults = db->exec(QString(
                    "SELECT sql FROM %1.%4 WHERE lower(name) = '%2' AND type = '%3';").arg(dbName, escapeString(lowerName), typeStr, targetTable),
                    dbFlags
                );

    }
    else
    {
        queryResults = db->exec(QString(
                    "SELECT sql FROM %1.%3 WHERE lower(name) = '%2';").arg(dbName, escapeString(lowerName), targetTable),
                    dbFlags
                );
    }

    // Validate query results
    if (queryResults->isError())
    {
        qDebug() << "Could not get object's DDL:" << dbName << "." << lowerName << ", details:" << queryResults->getErrorText();
        return QString::null;
    }

    // The DDL string
    results = queryResults->getSingleCell();
    return results.toString();
}

QStringList SchemaResolver::getColumnsFromDdlUsingPragma(const QString& ddl)
{
    Parser parser(db->getDialect());
    if (!parser.parse(ddl) || parser.getQueries().isEmpty())
    {
        qWarning() << "Could not parse DDL for determinating columns using PRAGMA. The DDL was:\n" << ddl;
        return QStringList();
    }

    SqliteQueryPtr query = parser.getQueries().first();
    if (query->queryType == SqliteQueryType::CreateTable)
        return getColumnsUsingPragma(query.dynamicCast<SqliteCreateTable>().data());

    if (query->queryType == SqliteQueryType::CreateView)
        return getColumnsUsingPragma(query.dynamicCast<SqliteCreateView>().data());

    qWarning() << "Tried to get columns of DDL using pragma for statement other than table or view:" << sqliteQueryTypeToString(query->queryType) << "for DDL:\n" << ddl;
    return QStringList();
}

QStringList SchemaResolver::getColumnsUsingPragma(const QString& tableOrView)
{
    static_qstring(query, "PRAGMA table_info(%1)");
    SqlQueryPtr results = db->exec(query.arg(wrapObjIfNeeded(tableOrView, db->getDialect())));
    if (results->isError())
    {
        qWarning() << "Could not get column list using PRAGMA for table or view:" << tableOrView << ", error was:" << results->getErrorText();
        return QStringList();
    }

    QStringList cols;
    for (const SqlResultsRowPtr& row : results->getAll())
        cols << row->value("name").toString();

    return cols;
}

QStringList SchemaResolver::getColumnsUsingPragma(SqliteCreateTable* createTable)
{
    QString name = getUniqueName();
    SqliteCreateTable* stmt = dynamic_cast<SqliteCreateTable*>(createTable->clone());
    stmt->tempKw = true;
    stmt->table = name;
    stmt->database = QString();
    stmt->rebuildTokens();
    QString ddl = stmt->tokens.detokenize();
    delete stmt;

    SqlQueryPtr result = db->exec(ddl);
    if (result->isError())
    {
        qWarning() << "Could not create table for finding its columns using PRAGMA. Error was:" << result->getErrorText();
        return QStringList();
    }

    QStringList columns = getColumnsUsingPragma(name);

    static_qstring(dropSql, "DROP TABLE %1");
    db->exec(dropSql.arg(wrapObjIfNeeded(name, db->getDialect())));

    return columns;
}

QStringList SchemaResolver::getColumnsUsingPragma(SqliteCreateView* createView)
{
    QString name = getUniqueName();
    SqliteCreateView* stmt = dynamic_cast<SqliteCreateView*>(createView->clone());
    stmt->tempKw = true;
    stmt->view = name;
    stmt->database = QString();
    stmt->rebuildTokens();
    QString ddl = stmt->tokens.detokenize();
    delete stmt;

    SqlQueryPtr result = db->exec(ddl);
    if (result->isError())
    {
        qWarning() << "Could not create view for finding its columns using PRAGMA. Error was:" << result->getErrorText();
        return QStringList();
    }

    QStringList columns = getColumnsUsingPragma(name);

    static_qstring(dropSql, "DROP VIEW %1");
    db->exec(dropSql.arg(wrapObjIfNeeded(name, db->getDialect())));

    return columns;
}

SqliteQueryPtr SchemaResolver::getParsedObject(const QString &name, ObjectType type)
{
    return getParsedObject("main", name, type);
}

SqliteQueryPtr SchemaResolver::getParsedObject(const QString &database, const QString &name, ObjectType type)
{
    // Get DDL
    QString ddl = getObjectDdl(database, name, type);
    if (ddl.isNull())
        return SqliteQueryPtr();

    // Parse DDL
    return getParsedDdl(ddl);
}

StrHash< SqliteQueryPtr> SchemaResolver::getAllParsedObjects()
{
    return getAllParsedObjects("main");
}

StrHash< SqliteQueryPtr> SchemaResolver::getAllParsedObjects(const QString& database)
{
    return getAllParsedObjectsForType<SqliteQuery>(database, QString::null);
}

StrHash< SqliteCreateTablePtr> SchemaResolver::getAllParsedTables()
{
    return getAllParsedTables("main");
}

StrHash< SqliteCreateTablePtr> SchemaResolver::getAllParsedTables(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateTable>(database, "table");
}

StrHash< SqliteCreateIndexPtr> SchemaResolver::getAllParsedIndexes()
{
    return getAllParsedIndexes("main");
}

StrHash< SqliteCreateIndexPtr> SchemaResolver::getAllParsedIndexes(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateIndex>(database, "index");
}

StrHash< SqliteCreateTriggerPtr> SchemaResolver::getAllParsedTriggers()
{
    return getAllParsedTriggers("main");
}

StrHash< SqliteCreateTriggerPtr> SchemaResolver::getAllParsedTriggers(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateTrigger>(database, "trigger");
}

StrHash< SqliteCreateViewPtr> SchemaResolver::getAllParsedViews()
{
    return getAllParsedViews("main");
}

StrHash< SqliteCreateViewPtr> SchemaResolver::getAllParsedViews(const QString& database)
{
    return getAllParsedObjectsForType<SqliteCreateView>(database, "view");
}

SqliteQueryPtr SchemaResolver::getParsedDdl(const QString& ddl)
{
    if (!parser->parse(ddl))
    {
        qDebug() << "Could not parse DDL for parsing object by SchemaResolver. Errors are:";
        foreach (ParserError* err, parser->getErrors())
            qDebug() << err->getMessage();

        return SqliteQueryPtr();
    }

    // Validate parsed DDL
    QList<SqliteQueryPtr> queries = parser->getQueries();
    if (queries.size() == 0)
    {
        qDebug() << "No parsed query while getting temp table columns.";
        return SqliteQueryPtr();
    }

    // Preparing results
    return queries[0];
}

QStringList SchemaResolver::getObjects(const QString &type)
{
    return getObjects(QString::null, type);
}

QStringList SchemaResolver::getObjects(const QString &database, const QString &type)
{
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_NAMES, db, database, type);
    if (useCache && cache.contains(key))
        return cache.object(key, true)->toStringList();

    QStringList resList;
    QString dbName = getPrefixDb(database, db->getDialect());

    SqlQueryPtr results = db->exec(QString("SELECT name FROM %1.sqlite_master WHERE type = ?;").arg(dbName), {type}, dbFlags);

    QString value;
    foreach (SqlResultsRowPtr row, results->getAll())
    {
        value = row->value(0).toString();
        if (!isFilteredOut(value, type))
            resList << value;
    }

    if (useCache)
        cache.insert(key, new QVariant(resList));

    return resList;
}

QStringList SchemaResolver::getAllObjects()
{
    return getAllObjects(QString::null);
}

QStringList SchemaResolver::getAllObjects(const QString& database)
{
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_NAMES, db, database);
    if (useCache && cache.contains(key))
        return cache.object(key, true)->toStringList();

    QStringList resList;
    QString dbName = getPrefixDb(database, db->getDialect());

    SqlQueryPtr results = db->exec(QString("SELECT name, type FROM %1.sqlite_master;").arg(dbName), dbFlags);

    QString value;
    QString type;
    foreach (SqlResultsRowPtr row, results->getAll())
    {
        value = row->value("name").toString();
        type = row->value("type").toString();
        if (!isFilteredOut(value, type))
            resList << value;
    }

    if (useCache)
        cache.insert(key, new QVariant(resList));

    return resList;
}

QString SchemaResolver::getUniqueName(const QString& database, const QString& namePrefix, const QStringList& forbiddenNames)
{
    QStringList allObjects = getAllObjects(database);
    allObjects += forbiddenNames;
    QString baseName = namePrefix;
    QString name = baseName;
    for (int i = 0; allObjects.contains(name); i++)
        name = baseName + QString::number(i);

    return name;
}

QString SchemaResolver::getUniqueName(const QString& namePrefix, const QStringList& forbiddenNames)
{
    return getUniqueName("main", namePrefix, forbiddenNames);
}

QStringList SchemaResolver::getFkReferencingTables(const QString& table)
{
    return getFkReferencingTables("main", table);
}

QStringList SchemaResolver::getFkReferencingTables(const QString& database, const QString& table)
{
    Dialect dialect = db->getDialect();
    if (dialect == Dialect::Sqlite2)
        return QStringList();

    // Get all tables
    StrHash<SqliteCreateTablePtr> parsedTables = getAllParsedTables(database);

    // Exclude queried table from the list
    parsedTables.remove(table);

    // Resolve referencing tables
    return getFkReferencingTables(table, parsedTables.values());
}

QStringList SchemaResolver::getFkReferencingTables(const QString& table, const QList<SqliteCreateTablePtr>& allParsedTables)
{
    QStringList tables;

    QList<SqliteCreateTable::Constraint*> tableFks;
    QList<SqliteCreateTable::Column::Constraint*> fks;
    bool result = false;
    for (SqliteCreateTablePtr createTable : allParsedTables)
    {
        // Check table constraints
        tableFks = createTable->getForeignKeysByTable(table);
        result = contains<SqliteCreateTable::Constraint*>(tableFks, [&table](SqliteCreateTable::Constraint* fk)
        {
           return fk->foreignKey->foreignTable == table;
        });

        if (result)
        {
            tables << createTable->table;
            continue;
        }

        // Check column constraints
        for (SqliteCreateTable::Column* column : createTable->columns)
        {
            fks = column->getForeignKeysByTable(table);
            result = contains<SqliteCreateTable::Column::Constraint*>(fks, [&table](SqliteCreateTable::Column::Constraint* fk)
            {
                return fk->foreignKey->foreignTable == table;
            });

            if (result)
            {
                tables << createTable->table;
                break;
            }
        }
    }

    return tables;
}

QStringList SchemaResolver::getIndexesForTable(const QString& database, const QString& table)
{
    QStringList names;
    foreach (SqliteCreateIndexPtr idx, getParsedIndexesForTable(database, table))
        names << idx->index;

    return names;
}

QStringList SchemaResolver::getIndexesForTable(const QString& table)
{
    return getIndexesForTable("main", table);
}

QStringList SchemaResolver::getTriggersForTable(const QString& database, const QString& table)
{
    QStringList names;
    foreach (SqliteCreateTriggerPtr trig, getParsedTriggersForTable(database, table))
        names << trig->trigger;

    return names;
}

QStringList SchemaResolver::getTriggersForTable(const QString& table)
{
    return getTriggersForTable("main", table);
}

QStringList SchemaResolver::getTriggersForView(const QString& database, const QString& view)
{
    QStringList names;
    foreach (SqliteCreateTriggerPtr trig, getParsedTriggersForView(database, view))
        names << trig->trigger;

    return names;
}

QStringList SchemaResolver::getTriggersForView(const QString& view)
{
    return getTriggersForView("main", view);
}

QStringList SchemaResolver::getViewsForTable(const QString& database, const QString& table)
{
    QStringList names;
    foreach (SqliteCreateViewPtr view, getParsedViewsForTable(database, table))
        names << view->view;

    return names;
}

QStringList SchemaResolver::getViewsForTable(const QString& table)
{
    return getViewsForTable("main", table);
}

StrHash<SchemaResolver::ObjectDetails> SchemaResolver::getAllObjectDetails()
{
    return getAllObjectDetails("main");
}

StrHash<SchemaResolver::ObjectDetails> SchemaResolver::getAllObjectDetails(const QString& database)
{
    StrHash< ObjectDetails> details;
    ObjectDetails detail;
    QString type;

    QList<QVariant> rows;
    bool useCache = usesCache();
    ObjectCacheKey key(ObjectCacheKey::OBJECT_DETAILS, db, database);
    if (useCache && cache.contains(key))
    {
        rows = cache.object(key, true)->toList();
    }
    else
    {
        SqlQueryPtr results = db->exec(QString("SELECT name, type, sql FROM %1.sqlite_master").arg(getPrefixDb(database, db->getDialect())), dbFlags);
        if (results->isError())
        {
            qCritical() << "Error while getting all object details in SchemaResolver:" << results->getErrorCode();
            return details;
        }

        for (const SqlResultsRowPtr& row : results->getAll())
            rows << row->valueMap();

        if (useCache)
            cache.insert(key, new QVariant(rows));
    }

    QHash<QString, QVariant> row;
    for (const QVariant& rowVariant : rows)
    {
        row = rowVariant.toHash();
        type = row["type"].toString();
        detail.type = stringToObjectType(type);
        if (detail.type == ANY)
            qCritical() << "Unhlandled db object type:" << type;

        detail.ddl = row["sql"].toString();
        details[row["name"].toString()] = detail;
    }

    return details;
}

QList<SqliteCreateIndexPtr> SchemaResolver::getParsedIndexesForTable(const QString& database, const QString& table)
{
    QList<SqliteCreateIndexPtr> createIndexList;

    QStringList indexes = getIndexes(database);
    SqliteQueryPtr query;
    SqliteCreateIndexPtr createIndex;
    for (const QString& index : indexes)
    {
        if (index.startsWith("sqlite_", Qt::CaseInsensitive))
            continue;

        query = getParsedObject(database, index, INDEX);
        if (!query)
            continue;

        createIndex = query.dynamicCast<SqliteCreateIndex>();
        if (!createIndex)
        {
            qWarning() << "Parsed DDL was not a CREATE INDEX statement, while queried for indexes.";
            continue;
        }

        if (createIndex->table.compare(table, Qt::CaseInsensitive) == 0)
            createIndexList << createIndex;
    }
    return createIndexList;
}

QList<SqliteCreateIndexPtr> SchemaResolver::getParsedIndexesForTable(const QString& table)
{
    return getParsedIndexesForTable("main", table);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTable(const QString& database, const QString& table, bool includeContentReferences)
{
    return getParsedTriggersForTableOrView(database, table, includeContentReferences, true);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTable(const QString& table, bool includeContentReferences)
{
    return getParsedTriggersForTable("main", table, includeContentReferences);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForView(const QString& database, const QString& view, bool includeContentReferences)
{
    return getParsedTriggersForTableOrView(database, view, includeContentReferences, false);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForView(const QString& view, bool includeContentReferences)
{
    return getParsedTriggersForView("main", view, includeContentReferences);
}

QList<SqliteCreateTriggerPtr> SchemaResolver::getParsedTriggersForTableOrView(const QString& database, const QString& tableOrView,
                                                                        bool includeContentReferences, bool table)
{
    QList<SqliteCreateTriggerPtr> createTriggerList;

    QStringList triggers = getTriggers(database);
    SqliteQueryPtr query;
    SqliteCreateTriggerPtr createTrigger;
    foreach (const QString& trig, triggers)
    {
        query = getParsedObject(database, trig, TRIGGER);
        if (!query)
            continue;

        createTrigger = query.dynamicCast<SqliteCreateTrigger>();
        if (!createTrigger)
        {
            qWarning() << "Parsed DDL was not a CREATE TRIGGER statement, while queried for triggers." << createTrigger.data();
            continue;
        }

        // The condition below checks:
        // 1. if this is a call for table triggers and event time is INSTEAD_OF - skip this iteration
        // 2. if this is a call for view triggers and event time is _not_ INSTEAD_OF - skip this iteration
        // In other words, it's a logical XOR for "table" flag and "eventTime == INSTEAD_OF" condition.
        if (table == (createTrigger->eventTime == SqliteCreateTrigger::Time::INSTEAD_OF))
            continue;

        if (createTrigger->table.compare(tableOrView, Qt::CaseInsensitive) == 0)
            createTriggerList << createTrigger;
        else if (includeContentReferences && indexOf(createTrigger->getContextTables(), tableOrView, Qt::CaseInsensitive) > -1)
            createTriggerList << createTrigger;

    }
    return createTriggerList;
}

QString SchemaResolver::objectTypeToString(SchemaResolver::ObjectType type)
{
    switch (type)
    {
        case TABLE:
            return "table";
        case INDEX:
            return "index";
        case TRIGGER:
            return "trigger";
        case VIEW:
            return "view";
        case ANY:
            return QString();
    }
    return QString();
}

SchemaResolver::ObjectType SchemaResolver::stringToObjectType(const QString& type)
{
    if (type == "table")
        return SchemaResolver::TABLE;
    else if (type == "index")
        return SchemaResolver::INDEX;
    else if (type == "trigger")
        return SchemaResolver::TRIGGER;
    else if (type == "view")
        return SchemaResolver::VIEW;
    else
        return SchemaResolver::ANY;
}

void SchemaResolver::staticInit()
{
    cache.setExpireTime(3000);
}

bool SchemaResolver::usesCache()
{
    return db->getConnectionOptions().contains(USE_SCHEMA_CACHING) && db->getConnectionOptions()[USE_SCHEMA_CACHING].toBool();
}

QList<SqliteCreateViewPtr> SchemaResolver::getParsedViewsForTable(const QString& database, const QString& table)
{
    QList<SqliteCreateViewPtr> createViewList;

    QStringList views = getViews(database);
    SqliteQueryPtr query;
    SqliteCreateViewPtr createView;
    foreach (const QString& view, views)
    {
        query = getParsedObject(database, view, VIEW);
        if (!query)
            continue;

        createView = query.dynamicCast<SqliteCreateView>();
        if (!createView)
        {
            qWarning() << "Parsed DDL was not a CREATE VIEW statement, while queried for views.";
            continue;
        }

        if (indexOf(createView->getContextTables(), table, Qt::CaseInsensitive) > -1)
            createViewList << createView;
    }
    return createViewList;
}

QList<SqliteCreateViewPtr> SchemaResolver::getParsedViewsForTable(const QString& table)
{
    return getParsedViewsForTable("main", table);
}

void SchemaResolver::filterSystemIndexes(QStringList& indexes)
{
    Dialect dialect = db->getDialect();
    QMutableListIterator<QString> it(indexes);
    while (it.hasNext())
    {
        if (isSystemIndex(it.next(), dialect))
            it.remove();
    }
}

bool SchemaResolver::isWithoutRowIdTable(const QString& table)
{
    return isWithoutRowIdTable("main", table);
}

bool SchemaResolver::isWithoutRowIdTable(const QString& database, const QString& table)
{
    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return false;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
        return false;

    return !createTable->withOutRowId.isNull();
}

bool SchemaResolver::isVirtualTable(const QString& database, const QString& table)
{
    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return false;

    SqliteCreateVirtualTablePtr createVirtualTable = query.dynamicCast<SqliteCreateVirtualTable>();
    return !createVirtualTable.isNull();
}

bool SchemaResolver::isVirtualTable(const QString& table)
{
    return isVirtualTable("main", table);
}

SqliteCreateTablePtr SchemaResolver::resolveVirtualTableAsRegularTable(const QString& table)
{
    return resolveVirtualTableAsRegularTable("maine", table);
}

SqliteCreateTablePtr SchemaResolver::resolveVirtualTableAsRegularTable(const QString& database, const QString& table)
{
    return virtualTableAsRegularTable(database, table);
}

QStringList SchemaResolver::getWithoutRowIdTableColumns(const QString& table)
{
    return getWithoutRowIdTableColumns("main", table);
}

QStringList SchemaResolver::getWithoutRowIdTableColumns(const QString& database, const QString& table)
{
    QStringList columns;

    SqliteQueryPtr query = getParsedObject(database, table, TABLE);
    if (!query)
        return columns;

    SqliteCreateTablePtr createTable = query.dynamicCast<SqliteCreateTable>();
    if (!createTable)
        return columns;

    if (createTable->withOutRowId.isNull())
        return columns; // it's not WITHOUT ROWID table

    return createTable->getPrimaryKeyColumns();
}

QString SchemaResolver::getSqliteMasterDdl(bool temp)
{
    if (temp)
        return sqliteTempMasterDdl;

    return sqliteMasterDdl;
}

QStringList SchemaResolver::getCollations()
{
    QStringList list;
    if (db->getDialect() != Dialect::Sqlite3)
        return list;

    SqlQueryPtr results = db->exec("PRAGMA collation_list", dbFlags);
    if (results->isError())
    {
        qWarning() << "Could not read collation list from the database:" << results->getErrorText();
        return list;
    }

    SqlResultsRowPtr row;
    while (results->hasNext())
    {
        row = results->next();
        list << row->value("name").toString();
    }

    return list;
}

bool SchemaResolver::getIgnoreSystemObjects() const
{
    return ignoreSystemObjects;
}

void SchemaResolver::setIgnoreSystemObjects(bool value)
{
    ignoreSystemObjects = value;
}

bool SchemaResolver::getNoDbLocking() const
{
    return dbFlags.testFlag(Db::Flag::NO_LOCK);
}

void SchemaResolver::setNoDbLocking(bool value)
{
    if (value)
        dbFlags |= Db::Flag::NO_LOCK;
    else
        dbFlags ^= Db::Flag::NO_LOCK;
}


SchemaResolver::ObjectCacheKey::ObjectCacheKey(Type type, Db* db, const QString& value1, const QString& value2, const QString& value3) :
    type(type), db(db), value1(value1), value2(value2), value3(value3)
{
}

int qHash(const SchemaResolver::ObjectCacheKey& key)
{
    return qHash(key.type) ^ qHash(key.db) ^ qHash(key.value1) ^ qHash(key.value2) ^ qHash(key.value3);
}

int operator==(const SchemaResolver::ObjectCacheKey& k1, const SchemaResolver::ObjectCacheKey& k2)
{
    return (k1.type == k2.type && k1.db == k2.db && k1.value1 == k2.value1 && k1.value2 == k2.value2 && k1.value3 == k2.value3);
}
