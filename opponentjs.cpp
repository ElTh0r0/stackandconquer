/**
 * \file opponentjs.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2020 Thorsten Roth
 *
 * This file is part of StackAndConquer.
 *
 * StackAndConquer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StackAndConquer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StackAndConquer.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Interface to CPU script JS engine.
 */

#include "./opponentjs.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

OpponentJS::OpponentJS(const quint8 nID, const QPoint BoardDimensions,
                       const quint8 nHeightTowerWin, const QString &sOut,
                       const QString &sPad, QObject *parent)
  : QObject(parent),
    m_nID(nID),
    m_BoardDimensions(BoardDimensions),
    m_nHeightTowerWin(nHeightTowerWin),
    m_sOut(sOut),
    m_sPad(sPad),
    m_jsEngine(new QJSEngine(parent)) {
  m_obj = m_jsEngine->globalObject();
  m_obj.setProperty(QStringLiteral("cpu"), m_jsEngine->newQObject(this));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto OpponentJS::loadAndEvalCpuScript(const QString &sFilepath) -> bool {
  QFile f(sFilepath);
  if (!f.open(QFile::ReadOnly)) {
    qWarning() << "Couldn't open JS file:" << sFilepath;
    return false;
  }
  QString source = QString::fromUtf8(f.readAll());
  f.close();
  this->log("Script: " + sFilepath);

  QJSValue result(m_jsEngine->evaluate(source, sFilepath));
  if (result.isError()) {
    qCritical() << "Error in CPU" << m_nID << "script at line" <<
                   result.property(QStringLiteral("lineNumber")).toInt() <<
                   "\n" << result.toString();
    emit scriptError();
    return false;
  }

  // Check if makeMove() is available for calling the script
  if (!m_obj.hasProperty(QStringLiteral("makeMove")) ||
      !m_obj.property(QStringLiteral("makeMove")).isCallable()) {
    qCritical() << "Error in CPU" << m_nID << "script - function makeMove() " <<
                   "not found or not callable!";
    emit scriptError();
    return false;
  }

  m_obj.setProperty(QStringLiteral("nID"), m_nID);
  m_obj.setProperty(QStringLiteral("nBoardDimensionsX"), m_BoardDimensions.x());
  m_obj.setProperty(QStringLiteral("nBoardDimensionsY"), m_BoardDimensions.y());
  m_obj.setProperty(QStringLiteral("nHeightTowerWin"), m_nHeightTowerWin);
  m_obj.setProperty(QStringLiteral("sOut"), m_sOut);
  m_obj.setProperty(QStringLiteral("sPad"), m_sPad);
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void OpponentJS::makeMoveCpu(const QJsonArray &board,
                             const quint8 nPossibleMove) {
  // TODO(volunteer): Provide list with all possible moves
  // (without previous move reverted)
  QJsonDocument jsdoc(board);

  QString sJsBoard(jsdoc.toJson(QJsonDocument::Compact));
  m_obj.setProperty(QStringLiteral("jsboard"), sJsBoard);

  QJSValue result = m_obj.property(QStringLiteral("makeMove"))
                    .call(QJSValueList() << nPossibleMove);
  if (result.isError()) {
    qCritical() << "CPU" << m_nID <<
                   "- Error calling \"makeMove\" function at line:" <<
                   result.property(QStringLiteral("lineNumber")).toInt() <<
                   "\n" << result.toString();
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("CPU script execution error! "
                            "Please check the debug log."));
    emit scriptError();
  }
  // qDebug() << "Result of makeMove():" << result.toString();

  // CPU has to return an int array (single int = set stone, 3 int = move tower)
  if (result.isArray()) {
    if (1 == result.property(QStringLiteral("length")).toInt()) {  // Set stone
      if (result.property(0).isNumber()) {
        if (result.property(0).toInt() >= 0 &&
            result.property(0).toInt() < board.size()) {
          emit setStone(result.property(0).toInt(), false);
          return;
        }
      }
    }
    if (3 == result.property(QStringLiteral("length")).toInt()) {  // Move tower
      if (result.property(0).isNumber() &&
          result.property(1).isNumber() &&
          result.property(2).isNumber()) {
        if (result.property(0).toInt() >= 0 &&
            result.property(0).toInt() < board.size() &&
            result.property(1).toInt() > 0 &&
            result.property(2).toInt() >= 0 &&
            result.property(2).toInt() < board.size()) {
          emit moveTower(result.property(0).toInt(),   // From
                         result.property(1).toInt(),   // Number of stones
                         result.property(2).toInt());  // To
          return;
        }
      }
    }
  }

  qCritical() << "CPU" << m_nID << "script invalid return from makeMove():" <<
                 result.toString();
  QMessageBox::warning(nullptr, tr("Warning"),
                       tr("CPU script execution error! "
                          "Please check the debug log."));
  emit scriptError();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void OpponentJS::log(const QString &sMsg) {
  qDebug() << "CPU" << m_nID << "-" << sMsg;
}
