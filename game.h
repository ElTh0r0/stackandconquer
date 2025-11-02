// SPDX-FileCopyrightText: 2015-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GAME_H_
#define GAME_H_

#include <QJsonArray>
#include <QObject>
#include <QString>

class QGraphicsScene;
class QJsonObject;

class Board;
class Player;

#include "./settings.h"

class Game : public QObject {
  Q_OBJECT

 public:
  explicit Game(QWidget *pParent, QObject *pParentObj = nullptr);
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
