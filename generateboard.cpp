// SPDX-FileCopyrightText: 2022 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./generateboard.h"

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void GenerateBoard::startGeneration(const QStringList &cmdArgs,
                                    const QString &sBoardInExt,
                                    const QString &sBoardOutExt,
                                    const QString &sIN, const QString &sOUT) {
  GenerateBoard::loopFiles(GenerateBoard::checkCmdArgs(cmdArgs, sBoardInExt),
                           sBoardInExt, sBoardOutExt, sIN, sOUT);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto GenerateBoard::checkCmdArgs(const QStringList &cmdArgs,
                                 const QString &sBoardInExt)
    -> QPair<QFileInfo, QFileInfo> {
  if (cmdArgs.isEmpty()) {
    qWarning() << "Please specify input (can be file *" + sBoardInExt +
                      " or "
                      "folder) and optional the destination folder:";
    qWarning() << "./stackandconquer INPUT [DESTINATION]\n";
    exit(-1);
  }

  QFileInfo fiTest;
  fiTest.setFile(cmdArgs.at(0).trimmed());
  if (!fiTest.exists()) {
    qWarning() << "Input file/folder" << cmdArgs.at(0) << "cannot be found!\n";
    exit(-1);
  }

  QPair<QFileInfo, QFileInfo> fiInOut;
  fiInOut.first = QFileInfo(cmdArgs.at(0).trimmed());

  if (cmdArgs.size() > 1) {
    fiTest.setFile(cmdArgs.at(1));
    if (!fiTest.exists() || !fiTest.isDir()) {
      qWarning() << "Output folder" << cmdArgs.at(1) << "cannot be found!\n";
      exit(-1);
    }
    fiInOut.second = QFileInfo(cmdArgs.at(1).trimmed());
  }

  return fiInOut;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void GenerateBoard::loopFiles(const QPair<QFileInfo, QFileInfo> &fiInOut,
                              const QString &sBoardInExt,
                              const QString &sBoardOutExt, const QString &sIN,
                              const QString &sOUT) {
  QFile input;
  QFile output;
  if (fiInOut.first.isFile()) {
    if (QString(sBoardInExt).remove('.') != fiInOut.first.suffix()) {
      qWarning() << "Input file has to use " + sBoardInExt + " extension!\n";
      exit(-1);
    }
    input.setFileName(fiInOut.first.filePath());

    if (fiInOut.second.exists()) {
      output.setFileName(fiInOut.second.filePath() + "/" +
                         fiInOut.first.baseName() + sBoardOutExt);
    } else {
      QString sOut(fiInOut.first.filePath());
      sOut = sOut.replace(sBoardInExt, sBoardOutExt);
      output.setFileName(sOut);
    }

    if (!generateBoard(&input, &output, sIN, sOUT)) {
      exit(-1);
    }
  } else {  // Loop through input folder
    QDirIterator it(fiInOut.first.filePath(),
                    QStringList() << "*" + sBoardInExt,
                    QDir::NoDotAndDotDot | QDir::Files);
    while (it.hasNext()) {
      it.next();
      input.setFileName(it.filePath());

      if (fiInOut.second.exists()) {
        output.setFileName(fiInOut.second.filePath() + "/" +
                           it.fileInfo().baseName() + sBoardOutExt);
      } else {
        QString sOut(it.filePath());
        sOut = sOut.replace(sBoardInExt, sBoardOutExt);
        output.setFileName(sOut);
      }

      if (!generateBoard(&input, &output, sIN, sOUT)) {
        qWarning() << "Looping through input folder was interrupted!\n";
        exit(-1);
      }
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto GenerateBoard::generateBoard(QFile *pInput, QFile *pOutput,
                                  const QString &sIN, const QString &sOUT)
    -> bool {
  if (!pInput->open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Couldn't open input file for board generation:"
               << pInput->fileName() << "\n";
    return false;
  }
  qDebug() << "Input:\t" << pInput->fileName();

  QTextStream in(pInput);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  // Since Qt 6 UTF-8 is used by default
  in.setCodec("UTF-8");
#endif
  QList<QString> listBoard;
  QString sLine;
  bool bOk(true);
  quint16 nCountAllFields(0);
  int nRows(-1);
  int nColumns(0);
  int nTempCol(0);
  QList<uint> nPlayerStones;

  while (!in.atEnd()) {
    nRows++;
    sLine = in.readLine().trimmed();
    if (sLine.isEmpty()) {
      break;
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QStringList sList(sLine.split(',', QString::SkipEmptyParts));
#else
    const QStringList sList(sLine.split(',', Qt::SkipEmptyParts));
#endif
    if (sList.isEmpty()) {
      qWarning() << "Invalid line:" << nRows + 1;
      return false;
    }

    // FIRST LINE -------------------------------------------------------------
    // Contains number of stones for two/three/more player separated by ','
    // e.g. 20,15 -> two players = 20 stones, three players = 15 stones
    if (0 == nRows) {
      for (auto const &sNum : sList) {
        nPlayerStones.append(sNum.toUInt(&bOk));
        if (!bOk) {
          qWarning() << "ERROR: Invalid first line (stones players):" << sNum;
          return false;
        }
        if (nPlayerStones.last() == 0) {
          qWarning() << "ERROR: Invalid number of stones:" << sNum;
          return false;
        }
      }

      if (nPlayerStones.isEmpty()) {
        qWarning() << "ERROR: Invalid first line (stones players):" << sLine;
        return false;
      }
      continue;
    }

    // FURTHER LINES ----------------------------------------------------------
    nTempCol = 0;
    for (const auto &s : sList) {
      nCountAllFields++;
      nTempCol++;
      listBoard << s.trimmed();
      if (sIN != listBoard.last() && sOUT != listBoard.last()) {
        qWarning() << "ERROR: Board files contains invalid data (neither " +
                          sIN + " nor " + sOUT + ") in line"
                   << nRows + 1 << "at field:" << nTempCol;
        return false;
      }
    }
    if (1 == nRows) {
      nColumns = nTempCol;
    }

    // Check if all columns have the same length
    if (nColumns != nTempCol) {
      qWarning() << "ERROR: Line" << nRows + 1
                 << "has invalid length. "
                    "Expected length:"
                 << nColumns;
      return false;
    }
  }
  pInput->close();

  // CHECKS -------------------------------------------------------------------
  if (nRows * nColumns != nCountAllFields) {
    qWarning() << "ERROR: Invalid number of ALL fields:\n"
                  "Rows x cols ="
               << nRows * nColumns
               << "\nAll counted fields =" << nCountAllFields;
    return false;
  }

  // WRITE BOARD --------------------------------------------------------------
  if (!pOutput->open(QIODevice::WriteOnly)) {
    qWarning() << "ERROR: Couldn't open/write board file:"
               << pOutput->fileName();
    return false;
  }
  qDebug() << "Output:\t" << pOutput->fileName();

  // Convert board to json array
  QJsonArray jsonArray;
  const QList<QString> tmpBoard(listBoard);
  for (const auto &s : tmpBoard) {
    jsonArray << s;
  }
  QJsonArray stonesArray;
  for (auto nNum : nPlayerStones) {
    stonesArray << static_cast<int>(nNum);
  }

  QJsonObject jsonObj;
  jsonObj[QStringLiteral("Board")] = jsonArray;
  jsonObj[QStringLiteral("Rows")] = nRows;
  jsonObj[QStringLiteral("Columns")] = nColumns;
  jsonObj[QStringLiteral("PlayersStones")] = stonesArray;
  QJsonDocument jsDoc(jsonObj);

  if (-1 == pOutput->write(jsDoc.toJson())) {
    qWarning() << "ERROR: Error while writing output file!";
    return false;
  }
  pOutput->close();

  // INFO ---------------------------------------------------------------------
  QString sStones(QLatin1String(""));
  uint nPlayers = 1;
  for (auto nNum : nPlayerStones) {
    nPlayers++;
    sStones += " " + QString::number(nPlayers) + ":" + QString::number(nNum);
  }
  qDebug() << "\t Stones players:" << sStones;
  qDebug() << "\t Size:" << nRows << "x" << nColumns;
  qDebug() << "\t Number of all fields:" << nCountAllFields;
  qDebug() << "";

  return true;
}
