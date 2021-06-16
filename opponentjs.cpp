/**
 * \file opponentjs.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2021 Thorsten Roth
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
#include <QJSEngine>

OpponentJS::OpponentJS(const quint8 nID, const QPoint BoardDimensions,
                       const quint8 nHeightTowerWin, const quint8 nNumOfPlayers,
                       const QString &sOut, const QString &sPad,
                       QObject *parent)
  : QObject(parent),
    m_nID(nID),
    m_BoardDimensions(BoardDimensions),
    m_nHeightTowerWin(nHeightTowerWin),
    m_nNumOfPlayers(nNumOfPlayers),
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
    qCritical().noquote() << "Error in CPU P" + QString::number(m_nID) +
                             "script at line " +
                             result.property(
                               QStringLiteral("lineNumber")).toString() +
                             "\n         " + result.toString();
    emit scriptError();
    return false;
  }

  // Check if callCPU() is available for calling the script
  if (!m_obj.hasProperty(QStringLiteral("callCPU")) ||
      !m_obj.property(QStringLiteral("callCPU")).isCallable()) {
    qCritical() << "Error in CPU P" + QString::number(m_nID) +
                   "script - function callCPU() not found or not callable!";
    emit scriptError();
    return false;
  }

  m_obj.setProperty(QStringLiteral("nID"), m_nID);
  m_obj.setProperty(QStringLiteral("nBoardDimensionsX"), m_BoardDimensions.x());
  m_obj.setProperty(QStringLiteral("nBoardDimensionsY"), m_BoardDimensions.y());
  m_obj.setProperty(QStringLiteral("nHeightTowerWin"), m_nHeightTowerWin);
  m_obj.setProperty(QStringLiteral("nNumOfPlayers"), m_nNumOfPlayers);
  m_obj.setProperty(QStringLiteral("sOut"), m_sOut);
  m_obj.setProperty(QStringLiteral("sPad"), m_sPad);
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void OpponentJS::callJsCpu(const QJsonArray &board,
                           const QJsonDocument &legalMoves,
                           const qint8 nDirection) {
  QJsonDocument jsdoc(board);
  QString sJsonBoard(
        QString::fromLatin1(jsdoc.toJson(QJsonDocument::Compact)));
  QString sJsonMoves(
        QString::fromLatin1(legalMoves.toJson(QJsonDocument::Compact)));
  QJSValueList args;
  args << sJsonBoard << sJsonMoves << nDirection;

  QJSValue result = m_obj.property(QStringLiteral("callCPU")).call(args);
  if (result.isError()) {
    qCritical().noquote() << "CPU P" + QString::number(m_nID) +
                             "- Error calling \"callCPU\" function at line: " +
                             result.property(
                               QStringLiteral("lineNumber")).toString() +
                             "\n         " + result.toString();
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("CPU script execution error! "
                            "Please check the debug log."));
    emit scriptError();
  }
  // qDebug() << "Result of callCPU(): " + result.toString();

  // CPU has to return an int array with length 3
  QJsonArray move;
  if (result.isArray()) {
    if (3 == result.property(QStringLiteral("length")).toInt()) {
      if (result.property(0).isNumber() &&  // From (-1 --> set stone at "To")
          result.property(1).isNumber() &&  // Number of stones
          result.property(2).isNumber()) {  // To
        move << result.property(0).toInt() <<
                result.property(1).toInt() <<
                result.property(2).toInt();
        if (move.at(0).toInt() >= -1 && move.at(0).toInt() < board.size() &&
            move.at(1).toInt() > 0 &&
            move.at(2).toInt() >= 0 && move.at(2).toInt() < board.size()) {
          emit actionCPU(move);
          return;
        }
      }
    }
  }

  qCritical() << "CPU P" + QString::number(m_nID) +
                 "script invalid return from callCPU(): " + result.toString();
  QMessageBox::warning(nullptr, tr("Warning"),
                       tr("CPU script execution error! "
                          "Please check the debug log."));
  emit scriptError();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void OpponentJS::log(const QString &sMsg) {
  qDebug() << "CPU P" + QString::number(m_nID) + " - " + sMsg;
}
