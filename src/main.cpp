#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#include "vscp.h"
#include "version.h"
#include "vscphelper.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

#include "vscpworks.h"
#include "mainwindow.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
  spdlog::debug("Starting VSCP Works+");

  // Init pool
  spdlog::init_thread_pool(8192, 1);

  // Flush log every five seconds
  spdlog::flush_every(std::chrono::seconds(5));

  auto console = spdlog::stdout_color_mt("console");
  // Start out with level=info. Config may change this
  console->set_level(spdlog::level::trace);
  console->set_pattern("[vscpworks+: %c] [%^%l%$] %v");
  spdlog::set_default_logger(console);

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

  //////////////////////////////////////////////////////////////////////////////
  //                                spdlog
  //////////////////////////////////////////////////////////////////////////////

  // Console log
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  if (app.m_bEnableConsoleLog) {
    console_sink->set_level(app.m_consoleLogLevel);
    console_sink->set_pattern(app.m_consoleLogPattern);
  }
  else {
    // If disabled set to off
    console_sink->set_level(spdlog::level::off);
  }

  try {
    auto rotating_file_sink = 
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(app.m_fileLogPath,
                                                                    app.m_maxFileLogSize,
                                                                    app.m_maxFileLogFiles);
    if (app.m_bEnableFileLog) {
      rotating_file_sink->set_level(app.m_fileLogLevel);
      rotating_file_sink->set_pattern(app.m_fileLogPattern);
    }
    else {
      // If disabled set to off
      rotating_file_sink->set_level(spdlog::level::off);
    }

    std::vector<spdlog::sink_ptr> sinks{ console_sink, rotating_file_sink };
    auto logger = std::make_shared<spdlog::async_logger>("logger",
                                                         sinks.begin(),
                                                         sinks.end(),
                                                         spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);
    // The separate sub loggers will handle trace levels
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
  }
  catch (...) {
    console->critical("vscpd: Unable to init logsystem. Logs Exiting.");
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  //////////////////////////////////////////////////////////////////////////////
  //                                spdlog
  //////////////////////////////////////////////////////////////////////////////

  MainWindow mainWin; 
  if (!parser.positionalArguments().isEmpty()) {
      //mainWin.loadFile(parser.positionalArguments().first());
  }

  app.openVscpWorksDatabase();

  mainWin.show(); 
  return app.exec();

  // Save configuration
  app.writeSettings();
}
