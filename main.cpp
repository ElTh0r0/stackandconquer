// SPDX-FileCopyrightText: 2015-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QStandardPaths>
#include <QTextStream>
#include <QTime>

#include "./generateboard.h"
#include "./settings.h"
#include "./stackandconquer.h"

static QFile logfile;
static QTextStream out(&logfile);

void setupLogger(const QString &sDebugFilePath, const QString &sAppName,
                 const QString &sVersion);

void LoggingHandler(QtMsgType type, const QMessageLogContext &context,
                    const QString &sMsg);

auto main(int argc, char *argv[]) -> int {
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  QApplication::setStyle("Fusion");  // Supports dark scheme on Win 10/11
#endif

  QApplication app(argc, argv);
  app.setApplicationName(QStringLiteral(APP_NAME));
  app.setApplicationVersion(QStringLiteral(APP_VERSION));
  app.setApplicationDisplayName(QStringLiteral(APP_NAME));
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
  app.setWindowIcon(
      QIcon::fromTheme(QStringLiteral("stackandconquer"),
                       QIcon(QStringLiteral(":/stackandconquer.png"))));
  app.setDesktopFileName(QStringLiteral("com.github.elth0r0.stackandconquer"));
#endif

  QCommandLineParser cmdparser;
  cmdparser.setApplicationDescription(QStringLiteral(APP_DESC));
  cmdparser.addHelpOption();
  cmdparser.addVersionOption();
  QCommandLineOption enableDebug(QStringLiteral("debug"),
                                 QStringLiteral("Enable debug mode"));
  cmdparser.addOption(enableDebug);
  QCommandLineOption generateBoard(
      QStringLiteral("genboard"),
      QStringLiteral("Generate %1 from input file(s)")
          .arg(Settings::BOARD_FILE_EXT));
  cmdparser.addOption(generateBoard);

  cmdparser.addPositionalArgument(
      QStringLiteral("savegame"),
      QStringLiteral("Savegame file to be opened (*%1)")
          .arg(Settings::SAVE_FILE_EXT));
  cmdparser.addPositionalArgument(
      QStringLiteral("board_in"),
      QStringLiteral("Input file (*%1) or folder to be converted to "
                     "stackboard; only to be used with option --%2")
          .arg(Settings::BORD_IN_FILE_EXT, generateBoard.names().at(0)));
  cmdparser.addPositionalArgument(
      QStringLiteral("board_out"),
      QStringLiteral("Optional folder in which generated board files shall "
                     "be put; only to be used with option --%1")
          .arg(generateBoard.names().at(0)));
  cmdparser.process(app);

  if (cmdparser.isSet(generateBoard)) {
    GenerateBoard::startGeneration(cmdparser.positionalArguments(),
                                   Settings::BORD_IN_FILE_EXT.toLower(),
                                   Settings::BOARD_FILE_EXT.toLower(),
                                   Settings::FIELD_IN, Settings::FIELD_OUT);
    exit(0);
  }

  // Default share data path (Windows and debugging)
  QString sSharePath = app.applicationDirPath();
  // Standard installation path (Linux)
  QDir tmpDir(app.applicationDirPath() + "/../share/" +
              app.applicationName().toLower());
  if (!cmdparser.isSet(enableDebug) && tmpDir.exists()) {
    sSharePath = app.applicationDirPath() + "/../share/" +
                 app.applicationName().toLower();
  }
#if defined(Q_OS_OSX)
  sSharePath = app.applicationDirPath() + "/../Resources/";
#endif
  Settings::instance()->setSharePath(sSharePath);

  QStringList sListPaths =
      QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
  if (sListPaths.isEmpty()) {
    qCritical() << "Error while getting data standard path.";
    sListPaths << app.applicationDirPath();
  }
  const QDir userDataDir(sListPaths[0].toLower());

  // Create folder including possible parent directories (mkPATH)
  if (!userDataDir.exists()) {
    userDataDir.mkpath(userDataDir.absolutePath());
  }

  const QString sDebugFile(QStringLiteral("debug.log"));
  setupLogger(userDataDir.absolutePath() + "/" + sDebugFile,
              app.applicationName(), app.applicationVersion());

  if (cmdparser.isSet(enableDebug)) {
    qWarning() << "DEBUG mode enabled!";
  }

  StackAndConquer myStackAndConquer(userDataDir,
                                    cmdparser.positionalArguments());
  myStackAndConquer.show();
  int nRet = app.exec();

  logfile.close();
  return nRet;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void setupLogger(const QString &sDebugFilePath, const QString &sAppName,
                 const QString &sVersion) {
  // Remove old debug file
  if (QFile(sDebugFilePath).exists()) {
    QFile(sDebugFilePath).remove();
  }

  // Create new file
  logfile.setFileName(sDebugFilePath);
  if (!logfile.open(QIODevice::WriteOnly)) {
    qWarning() << "Couldn't create logging file: " + sDebugFilePath;
  } else {
    qInstallMessageHandler(LoggingHandler);
  }

  qDebug() << sAppName + " v" + sVersion;
  qDebug() << "Compiled with Qt" << QT_VERSION_STR;
  qDebug() << "Qt runtime" << qVersion();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void LoggingHandler(QtMsgType type, const QMessageLogContext &context,
                    const QString &sMsg) {
  QString sContext = sMsg + " (" + QString::fromLatin1(context.file) + ":" +
                     QString::number(context.line) + ", " +
                     QString::fromLatin1(context.function) + ")";
  QString sTime(QTime::currentTime().toString());

  switch (type) {
    case QtDebugMsg:
      out << sTime << " Debug: " << sMsg << "\n";
      out.flush();
      break;
    case QtWarningMsg:
      out << sTime << " Warning: " << sContext << "\n";
      out.flush();
      break;
    case QtCriticalMsg:
      out << sTime << " Critical: " << sContext << "\n";
      out.flush();
      break;
    case QtFatalMsg:
      out << sTime << " Fatal: " << sContext << "\n";
      out.flush();
      logfile.close();
      abort();
    default:
      out << sTime << " OTHER INFO: " << sContext << "\n";
      out.flush();
      break;
  }
}
