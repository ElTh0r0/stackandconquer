/**
 * \file CPlayer.h
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
 * Class definition for player.
 */

#ifndef STACKANDCONQUER_CPLAYER_H_
#define STACKANDCONQUER_CPLAYER_H_

#include <QString>

/**
 * \class CPlayer
 * \brief Player class.
 */
class CPlayer {
 public:
  CPlayer(bool bActive, bool bIsHuman, QString sName, quint8 nMaxStones);
  ~CPlayer();

  void setActive(bool bActive);
  bool getIsActive() const;
  bool getIsHuman() const;
  QString getName() const;
  void setStonesLeft(quint8 nStones);
  quint8 getStonesLeft() const;
  void increaseWonTowers();
  quint8 getWonTowers() const;
  void setCanMove(bool bCanMove);
  bool getCanMove() const;

 private:
  bool m_bIsActive;
  const bool m_bIsHuman;
  QString m_sName;
  const quint8 m_nMaxStones;
  quint8 m_nStonesLeft;
  quint8 m_nWonTowers;
  bool m_bCanMove;
};

#endif  // STACKANDCONQUER_CPLAYER_H_
