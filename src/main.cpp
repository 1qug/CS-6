#include "app/appinfo.h"
#include "ui/mainwindow/mainwindow.h"
#include "ui/theme.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(AppInfo::shortName());
    QCoreApplication::setApplicationVersion(AppInfo::version());
    QCoreApplication::setOrganizationName(AppInfo::organization());
    QGuiApplication::setApplicationDisplayName(AppInfo::productName());

    Theme::apply(app);

    MainWindow window;
    window.show();
    return app.exec();
}
