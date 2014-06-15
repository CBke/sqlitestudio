#include "exportmanager.h"
#include "services/pluginmanager.h"
#include "plugins/exportplugin.h"
#include "services/notifymanager.h"
#include "db/queryexecutor.h"
#include "exportworker.h"
#include <QThreadPool>
#include <QTextCodec>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFile>

ExportManager::ExportManager(QObject *parent) :
    PluginServiceBase(parent)
{
}

ExportManager::~ExportManager()
{
    safe_delete(config);
}

QStringList ExportManager::getAvailableFormats(ExportMode exportMode) const
{
    QStringList formats;
    for (ExportPlugin* plugin : PLUGINS->getLoadedPlugins<ExportPlugin>())
    {
        if (exportMode == UNDEFINED || plugin->getSupportedModes().testFlag(exportMode))
            formats << plugin->getFormatName();
    }

    return formats;
}

void ExportManager::configure(const QString& format, const StandardExportConfig& config)
{
    configure(format, new StandardExportConfig(config));
}

void ExportManager::configure(const QString& format, StandardExportConfig* config)
{
    if (exportInProgress)
    {
        qWarning() << "Tried to configure export while another export is in progress.";
        return;
    }

    plugin = getPluginForFormat(format);
    if (!plugin)
    {
        invalidFormat(format);
        return;
    }

    safe_delete(this->config);
    this->config = config;
}

bool ExportManager::isExportInProgress() const
{
    return exportInProgress;
}

void ExportManager::exportQueryResults(Db* db, const QString& query)
{
    if (!checkInitialConditions())
        return;

    if (!plugin->getSupportedModes().testFlag(QUERY_RESULTS))
    {
        notifyError(tr("Export plugin %1 doesn't support exporing query results.").arg(plugin->getFormatName()));
        return;
    }

    exportInProgress = true;
    mode = QUERY_RESULTS;

    ExportWorker* worker = prepareExport();
    if (!worker)
        return;

    worker->prepareExportQueryResults(db, query);
    QThreadPool::globalInstance()->start(worker);
}

void ExportManager::exportTable(Db* db, const QString& database, const QString& table)
{
    static const QString sql = QStringLiteral("SELECT * FROM %1");

    if (!checkInitialConditions())
        return;

    if (!plugin->getSupportedModes().testFlag(TABLE))
    {
        notifyError(tr("Export plugin %1 doesn't support exporing tables.").arg(plugin->getFormatName()));
        return;
    }

    exportInProgress = true;
    mode = TABLE;

    ExportWorker* worker = prepareExport();
    if (!worker)
        return;

    worker->prepareExportTable(db, database, table);
    QThreadPool::globalInstance()->start(worker);
}

void ExportManager::exportDatabase(Db* db, const QStringList& objectListToExport)
{
    if (!checkInitialConditions())
        return;

    if (!plugin->getSupportedModes().testFlag(DATABASE))
    {
        notifyError(tr("Export plugin %1 doesn't support exporing databases.").arg(plugin->getFormatName()));
        return;
    }

    exportInProgress = true;
    mode = DATABASE;

    ExportWorker* worker = prepareExport();
    if (!worker)
        return;

    worker->prepareExportDatabase(db, objectListToExport);
    QThreadPool::globalInstance()->start(worker);
}

void ExportManager::interrupt()
{
    emit orderWorkerToInterrupt();
}

ExportPlugin* ExportManager::getPluginForFormat(const QString& formatName) const
{
    for (ExportPlugin* plugin : PLUGINS->getLoadedPlugins<ExportPlugin>())
        if (plugin->getFormatName() == formatName)
            return plugin;

    return nullptr;
}

void ExportManager::invalidFormat(const QString& format)
{
    notifyError(tr("Export format '%1' is not supported. Supported formats are: %2.").arg(format).arg(getAvailableFormats().join(", ")));
}

bool ExportManager::checkInitialConditions()
{
    if (exportInProgress)
    {
        qWarning() << "Tried to call export while another export is in progress.";
        return false;
    }

    if (!plugin)
    {
        qWarning() << "Tried to call export while no export plugin was configured.";
        return false;
    }

    return true;
}

ExportWorker* ExportManager::prepareExport()
{
    QIODevice* output = getOutputStream();
    if (!output)
    {
        emit exportFailed();
        exportInProgress = false;
        return nullptr;
    }

    ExportWorker* worker = new ExportWorker(plugin, config, output);
    connect(worker, SIGNAL(finished(bool,QIODevice*)), this, SLOT(finalizeExport(bool,QIODevice*)));
    connect(this, SIGNAL(orderWorkerToInterrupt()), worker, SLOT(interrupt()));
    return worker;
}

void ExportManager::handleClipboardExport()
{
    if (plugin->getMimeType().isNull())
    {
        QString str = codecForName(config->codec)->toUnicode(bufferForClipboard->buffer());
        emit storeInClipboard(str);
    }
    else
        emit storeInClipboard(bufferForClipboard->buffer(), plugin->getMimeType());
}

void ExportManager::finalizeExport(bool result, QIODevice* output)
{
    if (result)
    {
        if (config->intoClipboard)
        {
            notifyInfo(tr("Export to the clipboard was successful."));
            handleClipboardExport();
        }
        else
            notifyInfo(tr("Export to the file '%1' was successful.").arg(config->outputFileName));

        emit exportSuccessful();
    }
    else
    {
        emit exportFailed();
    }
    emit exportFinished();

    output->close();
    delete output;

    bufferForClipboard = nullptr;
    exportInProgress = false;
}

QIODevice* ExportManager::getOutputStream()
{
    if (config->intoClipboard)
    {
        bufferForClipboard = new QBuffer();
        bufferForClipboard->open(QIODevice::WriteOnly);
        return bufferForClipboard;
    }
    else if (!config->outputFileName.trimmed().isEmpty())
    {
        QFile* file = new QFile(config->outputFileName);
        if (!file->open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            notifyError(tr("Could not export to file %1. File cannot be open for writting.").arg(config->outputFileName));
            delete file;
            return nullptr;
        }
        return file;
    }
    return nullptr;
}

bool ExportManager::isAnyPluginAvailable()
{
    return !PLUGINS->getLoadedPlugins<ExportPlugin>().isEmpty();
}
