#ifndef FORMMANAGER_H
#define FORMMANAGER_H

#include "mainwindow.h"
#include <QHash>
#include <QString>

class UiLoader;

class FormManager : public QObject
{
        Q_OBJECT

    public:
        FormManager();
        virtual ~FormManager();

        QWidget* createWidget(const QString& name);
        bool hasWidget(const QString& name);
        QStringList getAvailableForms() const;

    private:
        void init();
        void load();
        void loadRecurently(const QString& path, const QString& prefix = "");
        QString getWidgetName(const QString& path);
        QWidget* createWidgetByFullPath(const QString& path);

        UiLoader* uiLoader = nullptr;
        QHash<QString,QString> widgetNameToFullPath;
        QStringList resourceForms;

    private slots:
        void rescanResources(Plugin* plugin, PluginType* pluginType);
        void rescanResources(const QString& pluginName);
        void pluginsAboutToMassUnload();
        void pluginsInitiallyLoaded();
};

#define FORMS MainWindow::getInstance()->getFormManager()

#endif // FORMMANAGER_H
