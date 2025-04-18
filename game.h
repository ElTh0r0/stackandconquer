/**
 * \file game.h
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
 * Class definition game engine.
 */

#ifndef GAME_H_
#define GAME_H_

#include <QJsonArray>
#include <QObject>
#include <QString>

class QGraphicsScene;
class QJsonObject;

class Board;
class Player;
class Settings;

class Game : public QObject {
  Q_OBJECT

 public:
  explicit Game(QWidget *pParent, Settings *pSettings, const QString &sIN,
                const QString &sOUT, QObject *pParentObj = nullptr);
  ~Game();
  auto createGame(const QString &sSavegame = QLatin1String("")) -> bool;
  auto getScene() const -> QGraphicsScene *;
  auto saveGame(const QString &sFile) -> bool;
  void updatePlayers(bool bInitial = false, bool bDirectionChangesOnce = false);
  auto initCpu() -> bool;

 signals:
  void updateNames(const QStringList &sListName);
  void updateStones(const quint8 nID, const QString &sStones);
  void updateWon(const quint8 nID, const QString &sWon);
  void drawIcon(const quint8 nID);
  void setInteractive(bool bEnabled);
  void highlightActivePlayer(quint8 nActivePlayer, quint8 nPlayerWon = 0);
  void changeZoom();

 private slots:
  void makeMove(QJsonArray move);
  void caughtScriptError();

 private:
  void setStone(const int nIndex, const bool bDebug);
  void moveTower(const int nFrom, const quint8 nStones, const int nTo);
  static auto loadGame(const QString &sFile) -> QJsonObject;
  auto checkPossibleMoves() -> bool;
  auto checkMoveIsValid(const QJsonDocument &legalMoves, const QJsonArray &move)
      -> bool;
  void checkTowerWin(const int nIndex);
  void returnStones(const int nIndex);
  void delayCpu(const QList<int> &previousMove);

  QWidget *m_pParent;
  Settings *m_pSettings;
  QString m_sIN;
  QString m_sOUT;
  Board *m_pBoard;
  QString m_sBoardFile;
  quint8 m_nNumOfPlayers;
  QList<Player *> m_pPlayers;
  QJsonArray m_TowersNeededToWin;
  QJsonArray m_NumberOfStones;

  const quint8 m_nMaxTowerHeight;
  quint8 m_nTowersToWin;

  bool m_bScriptError;
  QList<int> m_previousMove;

  struct currentPlayer {
    quint8 ID;
    bool isHuman;
  } activePlayer;
};

#endif  // GAME_H_
