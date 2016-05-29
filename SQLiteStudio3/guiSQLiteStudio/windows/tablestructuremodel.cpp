#include "tablestructuremodel.h"
#include "iconmanager.h"
#include "common/unused.h"
#include "uiconfig.h"
#include <QFont>
#include <QDebug>
#include <QMimeData>

TableStructureModel::TableStructureModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int TableStructureModel::rowCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    if (createTable.isNull())
        return 0;

    return createTable->columns.size();
}

int TableStructureModel::columnCount(const QModelIndex& parent) const
{
    UNUSED(parent);
    if (createTable.isNull())
        return 0;

    switch (createTable->dialect)
    {
        case Dialect::Sqlite3:
            return 9;
        case Dialect::Sqlite2:
            return 7;
    }
    return 0;
}

QVariant TableStructureModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (createTable.isNull())
        return QVariant();

    if (!isValidColumnIdx(index.column()))
        return QVariant();

    int row = index.row();
    if (createTable->columns.size() <= row)
        return QVariant();

    switch (getHeaderColumn(index.column()))
    {
        case TableStructureModel::Columns::NAME:
        {
            if (role != Qt::DisplayRole)
                break;

            return getColumnName(row);
        }
        case TableStructureModel::Columns::TYPE:
        {
            if (role != Qt::DisplayRole)
                break;

            return getColumnType(row);
        }
        case TableStructureModel::Columns::PK:
        {
            if (role != Qt::DecorationRole)
                break;

            return getColumnPk(row);
        }
        case TableStructureModel::Columns::FK:
        {
            if (role != Qt::DecorationRole)
                break;

            return getColumnFk(row);
        }
        case TableStructureModel::Columns::UNIQUE:
        {
            if (role != Qt::DecorationRole)
                break;

            return getColumnUnique(row);
        }
        case TableStructureModel::Columns::CHECK:
        {
            if (role != Qt::DecorationRole)
                break;

            return getColumnCheck(row);
        }
        case TableStructureModel::Columns::NOTNULL:
        {
            if (role != Qt::DecorationRole)
                break;

            return getColumnNotNull(row);
        }
        case TableStructureModel::Columns::COLLATE:
        {
            if (role != Qt::DecorationRole)
                break;

            return getColumnCollate(row);
        }
        case TableStructureModel::Columns::DEFAULT:
        {
            if (role == Qt::FontRole)
                return getColumnDefaultFont(row);

            if (role == Qt::ForegroundRole)
                return getColumnDefaultColor(row);

            if (role == Qt::DisplayRole)
                return getColumnDefaultValue(row);

            break;
        }
    }
    return QVariant();
}

QVariant TableStructureModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Vertical)
            return section + 1;

        // Now it's horizontal orientation with DisplayRole
        return columnLabel(section);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

TableStructureModel::Columns TableStructureModel::getHeaderColumn(int colIdx) const
{
    if (!createTable.isNull() && createTable->dialect == Dialect::Sqlite2)
    {
        if (colIdx >= 3)
            colIdx++; // skip FK

        if (colIdx >= 7)
            colIdx++; // skip COLLATE
    }
    return static_cast<Columns>(colIdx);
}

bool TableStructureModel::isValidColumnIdx(int colIdx) const
{
    if (!createTable.isNull() && createTable->dialect == Dialect::Sqlite2)
        return colIdx >= 0 && colIdx < 7;

    return colIdx >= 0 && colIdx < 9;
}

SqliteCreateTable::Column* TableStructureModel::getColumn(int colIdx) const
{
    if (createTable.isNull())
        return nullptr;

    return createTable->columns[colIdx];
}

void TableStructureModel::replaceColumn(int colIdx, SqliteCreateTable::Column* column)
{
    if (createTable.isNull())
        return;

    SqliteCreateTable::Column* oldColumn = createTable->columns[colIdx];
    QString oldColumnName = oldColumn->name;

    delete oldColumn;
    createTable->columns[colIdx] = column;
    column->setParent(createTable);
    modified = true;

    emit modifiyStateChanged();
    emit dataChanged(createIndex(colIdx, 0), createIndex(colIdx, columnCount()-1));
    emit columnModified(oldColumnName, column);
}

void TableStructureModel::insertColumn(int colIdx, SqliteCreateTable::Column* column)
{
    if (createTable.isNull())
        return;

    beginInsertRows(QModelIndex(), colIdx, colIdx);
    createTable->columns.insert(colIdx, column);
    column->setParent(createTable);
    endInsertRows();

    modified = true;
    emit modifiyStateChanged();
}

void TableStructureModel::appendColumn(SqliteCreateTable::Column* column)
{
    if (createTable.isNull())
        return;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    createTable->columns.append(column);
    column->setParent(createTable);
    endInsertRows();

    modified = true;
    emit modifiyStateChanged();
}

void TableStructureModel::delColumn(int colIdx)
{
    if (createTable.isNull())
        return;

    QString name = createTable->columns[colIdx]->name;

    beginRemoveRows(QModelIndex(), colIdx, colIdx);
    delete createTable->columns[colIdx];
    createTable->columns.removeAt(colIdx);
    endRemoveRows();

    modified = true;
    emit modifiyStateChanged();
    emit columnDeleted(name);
}

void TableStructureModel::moveColumnUp(int colIdx)
{
    moveColumnTo(colIdx, colIdx-1);
}

void TableStructureModel::moveColumnDown(int colIdx)
{
    moveColumnTo(colIdx, colIdx+1);
}

void TableStructureModel::moveColumnTo(int colIdx, int newIdx)
{
    if (createTable.isNull())
        return;

    if (newIdx == colIdx)
        return;

    int totalCols = createTable->columns.size();
    if (colIdx + 1 == totalCols && newIdx == totalCols) // Moving last column out of range? Nothing to do.
        return;

    if (newIdx == colIdx+1)
    {
        // From Qt docs: "you must ensure that the destinationChild is not within the range of sourceFirst and sourceLast + 1".
        // So in this case - which is easy to handle - we will invert operation. We will move target index one level up,
        // instead of moving source index down.
        int tmpIdx = newIdx;
        newIdx = colIdx;
        colIdx = tmpIdx;
    }

    beginMoveRows(QModelIndex(), colIdx, colIdx, QModelIndex(), newIdx);
    if (newIdx >= totalCols)
    {
        SqliteCreateTable::Column* col = createTable->columns.takeAt(colIdx);
        createTable->columns.append(col);
    }
    else
        createTable->columns.move(colIdx, newIdx);

    endMoveRows();

    modified = true;
    emit modifiyStateChanged();
    emit columnsOrderChanged();
}

QModelIndex TableStructureModel::findColumn(const QString& columnName, Qt::CaseSensitivity cs) const
{
    int row = 0;
    foreach (SqliteCreateTable::Column* col, createTable->columns)
    {
        if (col->name.compare(columnName, cs) == 0)
            return createIndex(row, 0);

        row++;
    }
    return QModelIndex();
}

QString TableStructureModel::columnLabel(int column) const
{
    switch (getHeaderColumn(column))
    {
        case Columns::NAME:
            return tr("Name", "table structure columns");
        case Columns::TYPE:
            return tr("Data type", "table structure columns");
        case Columns::PK:
            return tr("Primary\nKey", "table structure columns");
        case Columns::FK:
            return tr("Foreign\nKey", "table structure columns");
        case Columns::UNIQUE:
            return tr("Unique", "table structure columns");
        case Columns::CHECK:
            return tr("Check", "table structure columns");
        case Columns::NOTNULL:
            return tr("Not\nNULL", "table structure columns");
        case Columns::COLLATE:
            return tr("Collate", "table structure columns");
        case Columns::DEFAULT:
            return tr("Default value", "table structure columns");
    }
    return QString::null;
}

QVariant TableStructureModel::getColumnName(int row) const
{
    return getColumn(row)->name;
}

QVariant TableStructureModel::getColumnType(int row) const
{
    SqliteColumnType* type = getColumn(row)->type;
    return type ? type->detokenize() : "";
}

QVariant TableStructureModel::getColumnPk(int row) const
{
    if (isColumnPk(getColumn(row)))
        return ICONS.CONSTRAINT_PRIMARY_KEY;

    return QVariant();
}

QVariant TableStructureModel::getColumnFk(int row) const
{
    if (isColumnFk(getColumn(row)))
        return ICONS.CONSTRAINT_FOREIGN_KEY;

    return QVariant();
}

QVariant TableStructureModel::getColumnUnique(int row) const
{
    if (isColumnUnique(getColumn(row)))
        return ICONS.CONSTRAINT_UNIQUE;

    return QVariant();
}

QVariant TableStructureModel::getColumnCheck(int row) const
{
    if (isColumnCheck(getColumn(row)))
        return ICONS.CONSTRAINT_CHECK;

    return QVariant();
}

QVariant TableStructureModel::getColumnNotNull(int row) const
{
    if (isColumnNotNull(getColumn(row)))
        return ICONS.CONSTRAINT_NOT_NULL;

    return QVariant();
}

QVariant TableStructureModel::getColumnCollate(int row) const
{
    if (isColumnCollate(getColumn(row)))
        return ICONS.CONSTRAINT_COLLATION;

    return QVariant();
}

QVariant TableStructureModel::getColumnDefaultValue(int row) const
{
    QVariant value = getColumnDefault(row);
    if (value.isNull())
        return "NULL";

    return value;
}

QVariant TableStructureModel::getColumnDefaultFont(int row) const
{
    QVariant value = getColumnDefault(row);
    if (value.isNull())
    {
        QFont font;
        font.setItalic(true);
        return font;
    }
    return QVariant();
}

QVariant TableStructureModel::getColumnDefaultColor(int row) const
{
    QVariant value = getColumnDefault(row);
    if (value.isNull())
        return QColor(CFG_UI.Colors.DataNullFg);

    return QVariant();
}

QVariant TableStructureModel::getColumnDefault(int row) const
{
    SqliteCreateTable::Column::Constraint* constr = getColumn(row)->getConstraint(SqliteCreateTable::Column::Constraint::DEFAULT);
    if (!constr)
        return QVariant();

    if (!constr->id.isNull())
        return constr->id;
    else if (!constr->literalValue.isNull())
        return constr->literalValue;
    else if (!constr->ctime.isNull())
        return constr->ctime;
    else if (constr->expr)
        return constr->expr->detokenize();
    else
        return QVariant();
}

bool TableStructureModel::isColumnPk(SqliteCreateTable::Column* column) const
{
    if (column->hasConstraint(SqliteCreateTable::Column::Constraint::PRIMARY_KEY))
        return true;

    QList<SqliteCreateTable::Constraint*> constraints = createTable->getConstraints(SqliteCreateTable::Constraint::PRIMARY_KEY);
    foreach (SqliteCreateTable::Constraint* constr, constraints)
        if (constr->doesAffectColumn(column->name))
            return true;

    return false;
}

bool TableStructureModel::isColumnFk(SqliteCreateTable::Column* column) const
{
    if (column->hasConstraint(SqliteCreateTable::Column::Constraint::FOREIGN_KEY))
        return true;

    QList<SqliteCreateTable::Constraint*> constraints = createTable->getConstraints(SqliteCreateTable::Constraint::FOREIGN_KEY);
    foreach (SqliteCreateTable::Constraint* constr, constraints)
        if (constr->doesAffectColumn(column->name))
            return true;

    return false;
}

bool TableStructureModel::isColumnUnique(SqliteCreateTable::Column* column) const
{
    if (column->hasConstraint(SqliteCreateTable::Column::Constraint::UNIQUE))
        return true;

    QList<SqliteCreateTable::Constraint*> constraints = createTable->getConstraints(SqliteCreateTable::Constraint::UNIQUE);
    foreach (SqliteCreateTable::Constraint* constr, constraints)
        if (constr->doesAffectColumn(column->name))
            return true;

    return false;
}

bool TableStructureModel::isColumnCheck(SqliteCreateTable::Column* column) const
{
    if (column->hasConstraint(SqliteCreateTable::Column::Constraint::CHECK))
        return true;

    QList<SqliteCreateTable::Constraint*> constraints = createTable->getConstraints(SqliteCreateTable::Constraint::CHECK);
    foreach (SqliteCreateTable::Constraint* constr, constraints)
        if (constr->expr->getContextColumns(false).contains(column->name, Qt::CaseInsensitive))
            return true;

    return false;
}

bool TableStructureModel::isColumnNotNull(SqliteCreateTable::Column* column) const
{
    if (column->hasConstraint(SqliteCreateTable::Column::Constraint::NOT_NULL))
        return true;

    return false;
}

bool TableStructureModel::isColumnCollate(SqliteCreateTable::Column* column) const
{
    if (column->hasConstraint(SqliteCreateTable::Column::Constraint::COLLATE))
        return true;

    return false;
}

void TableStructureModel::setCreateTable(SqliteCreateTable* value)
{
    beginResetModel();
    createTable = value;
    endResetModel();

    modified = false;
    emit modifiyStateChanged();
}

bool TableStructureModel::isModified() const
{
    return modified;
}

Qt::DropActions TableStructureModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions TableStructureModel::supportedDragActions() const
{
    return Qt::CopyAction|Qt::MoveAction;
}


QStringList TableStructureModel::mimeTypes() const
{
    return {mimeType};
}

QMimeData* TableStructureModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.size() < 1)
        return nullptr;

    QModelIndex idx = indexes.first();

    QMimeData *data = new QMimeData();

    QByteArray output;
    QDataStream stream(&output, QIODevice::WriteOnly);

    stream << idx.row();
    data->setData(mimeType, output);

    return data;
}


bool TableStructureModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    UNUSED(action);
    UNUSED(row);
    UNUSED(column);
    UNUSED(parent);

    if (!data)
        return false;

    if (!data->hasFormat(mimeType))
        return false;

    return true;
}

bool TableStructureModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    UNUSED(column);

    if (action == Qt::IgnoreAction)
        return true;

    if (!data)
        return false;

    if (!data->hasFormat(mimeType))
        return false;

    if (action != Qt::MoveAction)
        return false;

    if (row < 0)
    {
        if (!parent.isValid() && !createTable.isNull())
            row = createTable->columns.size();
        else
            row = parent.row();
    }

    if (row < 0)
        return false;

    QByteArray byteData = data->data(mimeType);
    QDataStream stream(&byteData, QIODevice::ReadOnly);
    int oldRow;
    stream >> oldRow;

    moveColumnTo(oldRow, row);
    return true;
}

Qt::ItemFlags TableStructureModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return defFlags|Qt::ItemIsDropEnabled;

    return defFlags|Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled;
}
