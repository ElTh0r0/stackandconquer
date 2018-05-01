/**
 * \file player.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2018 Thorsten Roth <elthoro@gmx.de>
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
 * Class definition for player.
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QString>

/**
 * \class Player
 * \brief Player class.
 */
class Player {
  public:
    Player(bool bActive, bool bIsHuman, QString &sName, quint8 nMaxStones);
    ~Player();

    void setActive(const bool bActive);
    bool getIsActive() const;
    bool getIsHuman() const;
    QString getName() const;
    void setStonesLeft(const quint8 nStones);
    quint8 getStonesLeft() const;
    void setWonTowers(const quint8 nWonTowers);
    quint8 getWonTowers() const;
    void setCanMove(const quint8 nCanMove);
    quint8 getCanMove() const;

  private:
    bool m_bIsActive;
    const bool m_bIsHuman;
    QString m_sName;
    const quint8 m_nMaxStones;
    quint8 m_nStonesLeft;
    quint8 m_nWonTowers;
    quint8 m_nCanMove;
};

#endif  // PLAYER_H_
