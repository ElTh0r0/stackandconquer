/**
 * \file COpponentJS.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2017 Thorsten Roth <elthoro@gmx.de>
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

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "./COpponentJS.h"

COpponentJS::COpponentJS(const quint8 nNumOfFields, const quint8 nHeightTowerWin,
                         QObject *parent)
  : QObject(parent),
    m_nNumOfFields(nNumOfFields),
    m_nHeightTowerWin(nHeightTowerWin),
    m_jsEngine(new QJSEngine(parent)) {
  m_obj = m_jsEngine->globalObject();
  m_obj.setProperty("cpu", m_jsEngine->newQObject(this));

  // TODO: C++ function accessible via CPU script for checking previous move reverted?
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool COpponentJS::loadAndEvalCpuScript(const QString &sFilepath) {
  QFile f(sFilepath);
  if (!f.open(QFile::ReadOnly)) {
    qWarning() << "Couldn't open JS file:" << sFilepath;
    return false;
  }
  QString source = QString::fromUtf8(f.readAll());
  f.close();
  qDebug() << "CPU script:" << sFilepath;

  QJSValue result(m_jsEngine->evaluate(source, sFilepath));
  if (result.isError()) {
    qCritical() << "Error in CPU script at line" <<
                   result.property("lineNumber").toInt() <<
                   "\n" << result.toString();
    emit scriptError();
    return false;
  }

  // Check if makeMove() is available for calling the script
  if (!m_obj.hasProperty("makeMove") ||
      !m_obj.property("makeMove").isCallable()) {
    qCritical() << "Error in CPU script - function makeMove() " <<
                   "not found or not callable!";
    emit scriptError();
    return false;
  }

  m_obj.setProperty("nNumOfFields", m_nNumOfFields);
  m_obj.setProperty("nHeightTowerWin", m_nHeightTowerWin);
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void COpponentJS::makeMoveCpu(const QList<QList<QList<quint8> > > board,
                              const bool bStonesLeft) {
  QJsonDocument jsdoc(this->convertBoardToJSON(board));

  QString sJsBoard(jsdoc.toJson(QJsonDocument::Compact));
  m_obj.setProperty("jsboard", sJsBoard);

  QJSValue result = m_obj.property("makeMove").call(QJSValueList() << bStonesLeft);
  if (result.isError()) {
    qCritical() << "Error calling \"makeMove\" function at line:" <<
                   result.property("lineNumber").toInt() <<
                   "\n" << result.toString();
    QMessageBox::warning(NULL, trUtf8("Warning"),
                         trUtf8("CPU script execution error! "
                                "Please check the debug log."));
    emit scriptError();
  }

  // qDebug() << "Result of makeMove():" << result.toString();
  QList<QPoint> listRet;
  listRet = this->evalMoveReturn(result.toString());
  // qDebug() << "RET" << listRet;

  if (1 == listRet.size()) {
    if (listRet[0].x() >= 0 && listRet[0].y() >= 0 &&
        listRet[0].x() < m_nNumOfFields && listRet[0].y() < m_nNumOfFields) {
      emit setStone(listRet[0]);
      return;
    }
  } else if (3 == listRet.size()) {
    if (listRet[0].x() >= 0 && listRet[0].y() >= 0 &&
        listRet[0].x() < m_nNumOfFields && listRet[0].y() < m_nNumOfFields &&
        listRet[1].x() >= 0 && listRet[1].y() >= 0 &&
        listRet[1].x() < m_nNumOfFields && listRet[1].y() < m_nNumOfFields &&
        listRet[2].x() > 0 && listRet[2].x() < m_nHeightTowerWin) {
      emit moveTower(listRet[0], listRet[1], quint8(listRet[2].x()));
      return;
    }
  }

  qCritical() << "CPU script invalid return value from makeMove():" <<
                 result.toString();
  QMessageBox::warning(NULL, trUtf8("Warning"),
                       trUtf8("CPU script execution error! "
                              "Please check the debug log."));
  emit scriptError();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QJsonDocument COpponentJS::convertBoardToJSON(
    const QList<QList<QList<quint8> > > board) {
  QJsonArray tower;
  QVariantList vartower;
  QJsonArray jsboard;

  for (int nRow = 0; nRow < m_nNumOfFields; nRow++) {
    QJsonArray line;
    for (int nCol = 0; nCol < m_nNumOfFields; nCol++) {
      vartower.clear();
      foreach (quint8 n, board[nRow][nCol]) {
        vartower << n;
      }
      tower = QJsonArray::fromVariantList(vartower);
      line.append(tower);
    }
    jsboard.append(line);
  }

  QJsonDocument jsdoc(jsboard);
  return jsdoc;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QList<QPoint> COpponentJS::evalMoveReturn(QString sReturn) {
  QList<QPoint> listReturn;
  QStringList sListRet;
  QStringList sListPoint;
  QPoint point;
  bool bOk1(true);
  bool bOk2(true);

  sListRet = sReturn.split("|");
  for (int i = 0; i < sListRet.size(); i++) {
    sListPoint.clear();

    sListPoint = sListRet[i].split(",");
    if (2 == sListPoint.size() && (0 == i || 1 == i)) {
      point.setX(sListPoint[0].trimmed().toInt(&bOk1, 10));
      point.setY(sListPoint[1].trimmed().toInt(&bOk2, 10));

      if (!bOk1 || !bOk2) {
        // In case of error, return empty list
        listReturn.clear();
        break;
      } else {
        listReturn.append(point);
      }
    } else if (2 == i) {  // Third value in list is only one int
      point.setX(sListRet[i].trimmed().toInt(&bOk1, 10));
      point.setY(-1);

      if (!bOk1) {
        listReturn.clear();
        break;
      } else {
        listReturn.append(point);
      }
    } else {
      listReturn.clear();
      break;
    }
  }

  return listReturn;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void COpponentJS::log(const QString &sMsg) const {
  qDebug() << sMsg;
}
