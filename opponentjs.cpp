/**
 * \file opponentjs.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2020 Thorsten Roth <elthoro@gmx.de>
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
 * along with StackAndConquer.  If not, see <http://www.gnu.org/licenses/>.
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

OpponentJS::OpponentJS(const quint8 nID, const QPoint NumOfFields,
                       const quint8 nHeightTowerWin, QObject *parent)
  : QObject(parent),
    m_nID(nID),
    m_NumOfFields(NumOfFields),
    m_nHeightTowerWin(nHeightTowerWin),
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
  qDebug() << "CPU" << m_nID << "script:" << sFilepath;

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
  m_obj.setProperty(QStringLiteral("nNumOfFieldsX"), m_NumOfFields.x());
  m_obj.setProperty(QStringLiteral("nNumOfFieldsY"), m_NumOfFields.y());
  m_obj.setProperty(QStringLiteral("nHeightTowerWin"), m_nHeightTowerWin);
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void OpponentJS::makeMoveCpu(const QList<QList<QList<quint8> > > &board,
                             const quint8 nPossibleMove) {
  // TODO(volunteer): Provide list with all possible moves
  // (without previous move reverted)
  QJsonDocument jsdoc(this->convertBoardToJSON(board));

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
  QList<QPoint> listRet;
  listRet = OpponentJS::evalMoveReturn(result.toString());
  // qDebug() << "RET" << listRet;

  if (1 == listRet.size()) {
    if (listRet[0].x() >= 0 && listRet[0].y() >= 0 &&
        listRet[0].x() < m_NumOfFields.x() &&
        listRet[0].y() < m_NumOfFields.y()) {
//    emit setStone(listRet[0], false);
      emit setStone(0, false);  // TODO(): Implement new board array
      return;
    }
  } else if (3 == listRet.size()) {
    if (listRet[0].x() >= 0 && listRet[0].y() >= 0 &&
        listRet[0].x() < m_NumOfFields.x() &&
        listRet[0].y() < m_NumOfFields.y() &&
        listRet[1].x() >= 0 && listRet[1].y() >= 0 &&
        listRet[1].x() < m_NumOfFields.x() &&
        listRet[1].y() < m_NumOfFields.y() &&
        listRet[2].x() > 0 && listRet[2].x() < m_nHeightTowerWin) {
//    emit moveTower(listRet[0], listRet[1], quint8(listRet[2].x()));
      emit moveTower(0, 0, 0);  // TODO(): Implement new board array
      return;
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

auto OpponentJS::convertBoardToJSON(
    const QList<QList<QList<quint8> > > &board) -> QJsonDocument {
  QJsonArray tower;
  QVariantList vartower;
  QJsonArray jsBoard;

  for (int nCol = 0; nCol < m_NumOfFields.x(); nCol++) {
    QJsonArray column;
    for (int nRow = 0; nRow < m_NumOfFields.y(); nRow++) {
      vartower.clear();
      foreach (quint8 n, board[nCol][nRow]) {
        vartower << n;
      }
      tower = QJsonArray::fromVariantList(vartower);
      column.append(tower);
    }
    jsBoard.append(column);
  }

  QJsonDocument jsDoc(jsBoard);
  return jsDoc;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto OpponentJS::evalMoveReturn(const QString &sReturn) -> QList<QPoint> {
  QList<QPoint> listReturn;
  QStringList sListRet;
  QStringList sListPoint;
  QPoint point;
  bool bOk1(true);
  bool bOk2(true);

  sListRet = sReturn.split('|');
  for (int i = 0; i < sListRet.size(); i++) {
    sListPoint.clear();

    sListPoint = sListRet[i].split(',');
    if (2 == sListPoint.size() && (0 == i || 1 == i)) {
      point.setX(sListPoint[0].trimmed().toInt(&bOk1, 10));
      point.setY(sListPoint[1].trimmed().toInt(&bOk2, 10));

      if (!bOk1 || !bOk2) {
        // In case of error, return empty list
        listReturn.clear();
        break;
      }
      listReturn.append(point);
    } else if (2 == i) {  // Third value in list is only one int
      point.setX(sListRet[i].trimmed().toInt(&bOk1, 10));
      point.setY(-1);

      if (!bOk1) {
        listReturn.clear();
        break;
      }
      listReturn.append(point);
    } else {
      listReturn.clear();
      break;
    }
  }

  return listReturn;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void OpponentJS::log(const QString &sMsg) {
  qDebug() << sMsg;
}
