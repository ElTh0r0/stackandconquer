/**
 * \file opponentjs.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-present Thorsten Roth
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

#ifndef OPPONENTJS_H_
#define OPPONENTJS_H_

#include <QJSValue>
#include <QJsonArray>
#include <QObject>
#include <QPoint>
#include <QVector>

class QJSEngine;

class OpponentJS : public QObject {
  Q_OBJECT

 public:
  explicit OpponentJS(const quint8 nID, const QPoint BoardDimensions,
                      const quint8 nHeightTowerWin, const quint8 nNumOfPlayers,
                      const QString &sOut, const QString &sPad,
                      QObject *pParent = nullptr);
  auto loadAndEvalCpuScript(const QString &sFilepath) -> bool;
  void callJsCpu(const QJsonArray &board, const QJsonDocument &legalMoves,
                 const qint8 nDirection, const QJsonArray &towersNeededToWin);

 public slots:
  void log(const QString &sMsg);
  quint8 getID();
  quint8 getNumOfPlayers();
  quint8 getHeightToWin();
  QJsonArray getTowersNeededToWin();
  int getBoardDimensionX();
  int getBoardDimensionY();
  QString getOutside();
  QString getPadding();

 signals:
  void actionCPU(QJsonArray move);
  void scriptError();

 private:
  const quint8 m_nID;
  const QPoint m_BoardDimensions;
  const quint8 m_nHeightTowerWin;
  const quint8 m_nNumOfPlayers;
  const QString m_sOut;
  const QString m_sPad;
  QJSEngine *m_jsEngine;
  QJSValue m_obj;
  QJsonArray m_TowersNeededToWin;
};

#endif  // OPPONENTJS_H_
