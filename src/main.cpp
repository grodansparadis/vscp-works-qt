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

  // // Init pool
  // spdlog::init_thread_pool(8192, 1);

  // // Flush log every five seconds
  // spdlog::flush_every(std::chrono::seconds(1));

  // The console log logs during the upstart phase
  // it is replaced by the main logger when system is
  // configured
  // auto console = spdlog::stdout_color_mt("console");
  // console->set_level(spdlog::level::trace);
  // console->set_pattern("[%c] [%^%l%$] %v");
  // spdlog::set_default_logger(console);

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
  // parser.addPositionalArgument("file", "The file to open.");
  QCommandLineOption targetDirectoryOption(QStringList() << "c"
                                                         << "config",
                                           QCoreApplication::translate("main", "Set <directory> as home."),
                                           QCoreApplication::translate("main", "directory"));
  parser.process(app);
  app.loadSettings();

  fprintf(stderr, "Application data loaded\n");

  //////////////////////////////////////////////////////////////////////////////
  //                                spdlog
  //////////////////////////////////////////////////////////////////////////////

  // patterns - https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  // https://github.com/gabime/spdlog/wiki/2.-Creating-loggers#creating-loggers-with-multiple-sinks

  try {

    // Init pool
    // spdlog::init_thread_pool(8192, 1);

    // Flush log every five seconds
    // spdlog::flush_every(std::chrono::seconds(1));

    // Console log
    // auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // if (app.m_bEnableConsoleLog) {
    //   console_sink->set_level(app.m_consoleLogLevel);
    //   console_sink->set_pattern(app.m_consoleLogPattern);
    // }
    // else {
    //   // If disabled set to off
    //   console_sink->set_level(spdlog::level::off);
    // }

    // File log
    // auto rotating_file_sink =
    //   std::make_shared<spdlog::sinks::rotating_file_sink_mt>(app.m_fileLogPath,
    //                                                          app.m_maxFileLogSize,
    //                                                          app.m_maxFileLogFiles);

    // if (app.m_bEnableFileLog) {
    //   rotating_file_sink->set_level(app.m_fileLogLevel);
    //   rotating_file_sink->set_pattern(app.m_fileLogPattern);
    // }
    // else {
    //   // If disabled set to off
    //   rotating_file_sink->set_level(spdlog::level::off);
    // }

    /*
    spdlog::init_thread_pool(1024 * 8, 1);
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(app.m_fileLogPath,
                                                                           app.m_maxFileLogSize,
                                                                           app.m_maxFileLogFiles));
    auto logger = std::make_shared<spdlog::async_logger>("log",
                                                         sinks.begin(),
                                                         sinks.end(),
                                                         spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::overrun_oldest);
    // The separate sub loggers will handle trace levels
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    //spdlog::initialize_logger(logger);
    */

    // logger->set_level(app.m_fileLogLevel);
    // logger->set_pattern(app.m_fileLogPattern);

    // console_sink->set_level(app.m_consoleLogLevel);
    // console_sink->set_pattern("%v" /*app.m_consoleLogPattern*/);

    // logger->sinks()[0]->set_level(app.m_consoleLogLevel);
    // logger->sinks()[0]->set_pattern(app.m_consoleLogPattern);

    // logger->sinks()[1]->set_level(app.m_fileLogLevel);
    // logger->sinks()[1]->set_pattern(app.m_fileLogPattern);

    // logger->sinks()[0]->set_level(spdlog::level::trace);
    // logger->sinks()[0]->set_pattern("test1 %v");

    // logger->sinks()[1]->set_level(spdlog::level::trace);
    // logger->sinks()[1]->set_pattern("test2 %v");

    //int size      =  app    10 * 1024 * 1024; // Max size of log file
    //int backcount = 10;                // Max # log files

    // create console_sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    // create rotating file sink
    auto file_sink = 
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>("~/.local/share/VSCP/vscpworks+/logs/log.txt", 
                                                                  app.m_maxFileLogSize, 
                                                                  app.m_maxFileLogFiles, 
                                                                  true);
    file_sink->set_level(spdlog::level::debug);

    // sink's bucket
    spdlog::sinks_init_list sinks{ console_sink, file_sink };

    // create async logger, and use global threadpool
    spdlog::init_thread_pool(1024 * 8, 1);
    auto logger = std::make_shared<spdlog::async_logger>("aslogger", sinks, spdlog::thread_pool());

    // set default logger
    spdlog::set_default_logger(logger);
    // spdlog::initialize_logger(logger);

    logger->sinks()[0]->set_level(app.m_consoleLogLevel);
    logger->sinks()[0]->set_pattern(app.m_consoleLogPattern);

    logger->sinks()[1]->set_level(app.m_fileLogLevel);
    logger->sinks()[1]->set_pattern(app.m_fileLogPattern);

    spdlog::info("VSCP Works + logging");
  }
  catch (...) {
    fprintf(stderr, "Unable to init logsystem. Logs Exiting.");
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  //////////////////////////////////////////////////////////////////////////////
  //                                spdlog
  //////////////////////////////////////////////////////////////////////////////

  spdlog::debug("Starting VSCP Works +");

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
      printf("Unable to set stylesheet, file not found\n");
    }
    else {
      f.open(QFile::ReadOnly | QFile::Text);
      QTextStream ts(&f);
      qApp->setStyleSheet(ts.readAll());
    }
  }

  mainWin.show();
  return app.exec();

  // Save configuration
  app.writeSettings();
}
