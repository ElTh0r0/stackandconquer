/**
 * \file CGame.h
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
 * Class definition game engine.
 */

#ifndef STACKANDCONQUER_CGAME_H_
#define STACKANDCONQUER_CGAME_H_

#include "./CBoard.h"
#include "./CPlayer.h"
#include "./COpponentJS.h"

class CGame : public QObject {
  Q_OBJECT

 public:
  explicit CGame(CSettings *pSettings, const QStringList &sListFiles);
  QGraphicsScene* getScene() const;
  QRectF getSceneRect() const;
  void updatePlayers(bool bInitial = false);
  bool initCpu();

 signals:
  void updateNameP1(QString sName);
  void updateNameP2(QString sName);
  void updateStonesP1(QString sStones);
  void updateStonesP2(QString sStones);
  void updateWonP1(QString sWon);
  void updateWonP2(QString sWon);
  void setInteractive(bool bEnabled);
  void highlightActivePlayer(bool bPlayer1,
                             bool bP1Won = false, bool bP2Won = false);
  void makeMoveCpuP1(QList<QList<QList<quint8> > > board, quint8 nPossibleMove);
  void makeMoveCpuP2(QList<QList<QList<quint8> > > board, quint8 nPossibleMove);

 private slots:
  void setStone(QPoint field);
  void moveTower(QPoint tower, QPoint moveTo, quint8 nStones = 0);
  void delayCpu();
  void caughtScriptError();

 private:
  void createCPU1();
  void createCPU2();
  void checkPossibleMoves();
  bool checkPreviousMoveReverted(const QString sMove);
  void checkTowerWin(QPoint field);
  void returnStones(QPoint field);

  CSettings *m_pSettings;
  CBoard *m_pBoard;
  COpponentJS *m_jsCpuP1;
  COpponentJS *m_jsCpuP2;
  CPlayer *m_pPlayer1;
  CPlayer *m_pPlayer2;
  QString m_sJsFileP1;
  QString m_sJsFileP2;

  const quint8 m_nMaxTowerHeight;
  const quint8 m_nMaxStones;
  const quint16 m_nGridSize;
  const quint8 m_nNumOfFields;

  bool m_bScriptError;
  QString m_sPreviousMove;
};

#endif  // STACKANDCONQUER_CGAME_H_
