#ifndef PDFEXPORT_H
#define PDFEXPORT_H

#include "pdfexport_global.h"
#include "plugins/genericexportplugin.h"

class QPdfWriter;
class QPainter;
class QTextOption;
class QFont;

class PDFEXPORTSHARED_EXPORT PdfExport : public GenericExportPlugin
{
        Q_OBJECT
        SQLITESTUDIO_PLUGIN("pdfexport.json")

    public:
        QString getFormatName() const;
        ExportManager::StandardConfigFlags standardOptionsToEnable() const;
        ExportManager::ExportProviderFlags getProviderFlags() const;
        void validateOptions();
        QString defaultFileExtension() const;
        bool beforeExportQueryResults(const QString& query, QList<QueryExecutor::ResultColumnPtr>& columns,
                                      const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportQueryResultsRow(SqlResultsRowPtr row);
        bool exportTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateTablePtr createTable,
                         const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool exportVirtualTable(const QString& database, const QString& table, const QStringList& columnNames, const QString& ddl, SqliteCreateVirtualTablePtr createTable,
                                const QHash<ExportManager::ExportProviderFlag,QVariant> providedData);
        bool afterExportTable();
        bool exportTableRow(SqlResultsRowPtr data);
        bool beforeExportDatabase(const QString& database);
        bool exportIndex(const QString& database, const QString& name, const QString& ddl, SqliteCreateIndexPtr createIndex);
        bool exportTrigger(const QString& database, const QString& name, const QString& ddl, SqliteCreateTriggerPtr createTrigger);
        bool exportView(const QString& database, const QString& name, const QString& ddl, SqliteCreateViewPtr view);
        bool afterExportQueryResults();
        bool afterExportDatabase();
        bool isBinaryData() const;
        bool init();
        void deinit();

    private:
        struct Cell
        {
            QString contents;
            Qt::Alignment alignment = Qt::AlignLeft;
            bool isNull = false;
            bool isRowNum = false;
        };

        struct Row
        {
            enum Type
            {
                NORMAL,
                HEADER,
                COLUMNS_HEADER
            };

            QList<Cell> cells;
            int height = 0;
            Type type = NORMAL;
        };

        void beginDoc(const QString& title);
        void endDoc();
        void setupConfig();
        void updateMargins();
        void drawDdl(const QString& objName, const QString& ddl);
        void clearHeaderRows();
        void exportDataRow(const QList<QVariant>& data);
        void checkForDataRender();
        void renderDataPages(bool finalRender = false);
        void renderDataRowsPage(int columnStart, int columnEndBefore, int rowsToRender);
        void renderDataRow(const Row& row, int& y, int columnStart, int columnEndBefore, int localRowNum);
        void renderDataCell(const QRect& rect, const Cell& cell);
        void renderDataCell(const QRect& rect, const QString& contents, QTextOption* opt);
        void renderHeaderRow(const Row& row, int& y, int totalColsWidth, int columnStart, int columnEndBefore);
        void renderHeaderCell(int& x, int y, const Row& row, int col, QTextOption* opt);
        void renderPageNumber();
        void exportObjectHeader(const QString& contents);
        void exportColumnsHeader(const QStringList& columns);
        void newPage();
        void calculateColumnWidths(const QStringList& columnNames, const QHash<ExportManager::ExportProviderFlag,QVariant> providedData, int columnToExpand = -1);
        void calculateRowHeights();
        int calculateRowHeight(int textWidth, const QString& contents);
        int getDataColumnsWidth() const;
        int getDataColumnsStartX() const;
        qreal mmToPoints(qreal sizeMM);

        QPdfWriter* pdfWriter = nullptr;
        QPainter* painter = nullptr;
        QTextOption* textOption = nullptr;
        QFont* stdFont = nullptr;
        QFont* boldFont = nullptr;
        QFont* italicFont = nullptr;
        int totalRows = 0;
        QList<Row> bufferedRows; // data rows
        int totalHeaderRowsHeight = 0; // total height of all current header rows
        int currentHeaderMinWidth = 0; // minimum width of the object header, required to extend data columns when hey are slimmer than this
        QList<int> calculatedColumnWidths; // column widths calculated basing on header column widths and data column widths
        QList<int> columnsPerPage; // number of columns that will fit on each page
        QScopedPointer<Row> headerRow; // Top level header (object name)
        QScopedPointer<Row> columnsHeaderRow; // columns header for data tables
        bool printRowNum = true;
        int rowNumColumnWidth = 0;
        int pageWidth = 0;
        int pageHeight = 0;
        int minRowHeight = 0;
        int maxRowHeight = 0;
        int maxColWidth = 0;
        int rowsToPrebuffer = 0;
        int padding = 0;
        int currentPage = -1;
        int rowNum = 0;
        int topMargin = 0;
        int rightMargin = 0;
        int leftMargin = 0;
        int bottomMargin = 0;
        qreal pointsPerMm = 1.0;
        int lineWidth = 15;
};

#endif // PDFEXPORT_H
