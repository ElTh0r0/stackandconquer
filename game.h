/**
 * \file game.h
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
 * Class definition game engine.
 */

#ifndef GAME_H_
#define GAME_H_

#include "./board.h"
#include "./player.h"
#include "./opponentjs.h"

class Game : public QObject {
  Q_OBJECT

 public:
    explicit Game(Settings *pSettings, const QStringList &sListFiles);
    ~Game();
    auto getScene() const -> QGraphicsScene*;
    auto saveGame(const QString &sFile) -> bool;
    void updatePlayers(bool bInitial = false);
    auto initCpu() -> bool;

 signals:
    void updateNameP1(const QString &sName);
    void updateNameP2(const QString &sName);
    void updateStonesP1(const QString &sStones);
    void updateStonesP2(const QString &sStones);
    void updateWonP1(const QString &sWon);
    void updateWonP2(const QString &sWon);
    void setInteractive(bool bEnabled);
    void highlightActivePlayer(bool bPlayer1,
                               bool bP1Won = false, bool bP2Won = false);
    void makeMoveCpuP1(const QJsonArray &board, const quint8 nPossibleMove);
    void makeMoveCpuP2(const QJsonArray &board, const quint8 nPossibleMove);

 private slots:
    void setStone(int nIndex, bool bDebug);
    void moveTower(int nFrom, quint8 nStones, int nTo);
    void delayCpu();
    void caughtScriptError();

 private:
    void createCPU1();
    void createCPU2();
    static auto loadGame(const QString &sFile) -> QJsonObject;
    auto checkPossibleMoves() -> bool;
    auto checkPreviousMoveReverted(const QString &sMove) -> bool;
    void checkTowerWin(const int nIndex);
    void returnStones(const int nIndex);

    Settings *m_pSettings;
    Board *m_pBoard;
    QString m_sBoardFile;
    OpponentJS *m_jsCpuP1;
    OpponentJS *m_jsCpuP2;
    Player *m_pPlayer1;
    Player *m_pPlayer2;
    QString m_sJsFileP1;
    QString m_sJsFileP2;

    const quint8 m_nMaxTowerHeight;
    const quint16 m_nGridSize;

    bool m_bScriptError;
    QString m_sPreviousMove;
};

#endif  // GAME_H_
