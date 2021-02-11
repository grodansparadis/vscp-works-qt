#include "vscp.h"
#include "version.h"
#include "vscphelper.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

#include "vscpworks.h"
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    vscpworks app(argc, argv);
    QCoreApplication::setOrganizationName("VSCP");
    QCoreApplication::setOrganizationDomain("vscp.org");
    QCoreApplication::setApplicationName("vscpworks+");
    QCoreApplication::setApplicationVersion(VSCPWORKS_DISPLAY_VERSION);

    // vscpworks --config "config folder"
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    //parser.addPositionalArgument("file", "The file to open.");
    QCommandLineOption targetDirectoryOption(QStringList() << "c" << "config",
            QCoreApplication::translate("main", "Set <directory> as home."),
            QCoreApplication::translate("main", "directory"));
    parser.process(app);

    app.loadSettings();

    MainWindow mainWin; 
    if (!parser.positionalArguments().isEmpty()) {
        //mainWin.loadFile(parser.positionalArguments().first());
    }

    mainWin.show(); 
    return app.exec();

    // Save configuration
    app.writeSettings();
}
