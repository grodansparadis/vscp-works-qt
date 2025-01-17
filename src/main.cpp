#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include "version.h"
#include "vscp.h"
#include "vscphelper.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

#include "mainwindow.h"
#include "vscpworks.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int
main(int argc, char* argv[])
{
  vscpworks app(argc, argv);
  QCoreApplication::setOrganizationName("VSCP");
  QCoreApplication::setOrganizationDomain("vscp.org");
  QCoreApplication::setApplicationName("vscpworks+");
  QCoreApplication::setApplicationVersion(VSCPWORKS_DISPLAY_VERSION);

  // Go to users home directory
  QDir::setCurrent(QDir::homePath());

  // vscpworks --config "config folder"
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addVersionOption();
  // parser.addPositionalArgument("file", "The file to open.");
  QCommandLineOption targetDirectoryOption(QStringList() << "c"
                                                         << "config",
                                           QCoreApplication::translate("main", "Set <directory> as home."),
                                           QCoreApplication::translate("main", "directory"));
  parser.process(app);
  app.loadSettings();

  fprintf(stderr, "Application data loaded\n");

  //////////////////////////////////////////////////////////////////////////////
  //                                <spdlog>
  //////////////////////////////////////////////////////////////////////////////

  // patterns - https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  // https://github.com/gabime/spdlog/wiki/2.-Creating-loggers#creating-loggers-with-multiple-sinks

  try {

    // create console_sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(app.m_consoleLogLevel);
    console_sink->set_pattern(app.m_consoleLogPattern);

    // create rotating file sink
    auto file_sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(app.m_fileLogPath,
                                                             app.m_maxFileLogSize,
                                                             app.m_maxFileLogFiles,
                                                             true);
    file_sink->set_level(app.m_fileLogLevel);
    file_sink->set_pattern(app.m_fileLogPattern);

    // sink's bucket
    spdlog::sinks_init_list sinks{ console_sink, file_sink };

    // create async logger, and use global threadpool
    spdlog::init_thread_pool(1024 * 8, 1);
    auto logger = std::make_shared<spdlog::async_logger>("xlogger", sinks, spdlog::thread_pool());

    // set default logger
    spdlog::set_default_logger(logger);
    // spdlog::initialize_logger(logger);

    //logger->sinks()[0]->set_level(app.m_consoleLogLevel);
    //logger->sinks()[0]->set_pattern(app.m_consoleLogPattern);

    //logger->sinks()[1]->set_level(app.m_fileLogLevel);
    //logger->sinks()[1]->set_pattern(app.m_fileLogPattern);

    spdlog::set_level(app.m_fileLogLevel);
    spdlog::debug("VSCP Works + logging");
  }
  catch (const spdlog::spdlog_ex &ex) {
    fprintf(stderr, "Unable to init logsystem. Logs Exiting.\n");
    std::cout << "Log init failed: " << ex.what() << std::endl;
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  //////////////////////////////////////////////////////////////////////////////
  //                                </spdlog>
  //////////////////////////////////////////////////////////////////////////////

  spdlog::info("Starting VSCP Works +");

  MainWindow mainWin;
  if (!parser.positionalArguments().isEmpty()) {
    // mainWin.loadFile(parser.positionalArguments().first());
  }

  app.openVscpWorksDatabase();

  // https://github.com/ColinDuquesnoy/QDarkStyleSheet
  if (0 && app.m_bEnableDarkTheme) {
    // mainWin.setStyleSheet(QString::fromUtf8(qdarkstyle::load_stylesheet_from_file(":/qdarkstyle/style.qss").toStdString().c_str()));

    QFile f(":style.qss");

    if (!f.exists()) {
      fprintf(stderr,"Unable to set stylesheet, file not found\n");
    }
    else {
      f.open(QFile::ReadOnly | QFile::Text);
      QTextStream ts(&f);
      qApp->setStyleSheet(ts.readAll());
    }
  }

  mainWin.show();
  return app.exec();
}
