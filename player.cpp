/**
 * \file player.cpp
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
 * Player.
 */

#include "./player.h"

#include <QDebug>
#include <QMessageBox>

Player::Player(bool bActive, bool bIsHuman,
               const QString &sName, quint8 nMaxStones)
  : m_bIsActive(bActive),
    m_bIsHuman(bIsHuman),
    m_sName(sName),
    m_nMaxStones(nMaxStones),
    m_nStonesLeft(nMaxStones),
    m_nWonTowers(0),
    m_nCanMove(0) {
  if (!m_bIsHuman) {
    m_sName = QStringLiteral("Computer");
  }
  qDebug() << "Generate player" << m_sName;
}

Player::~Player() = default;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::setActive(const bool bActive) {
  m_bIsActive = bActive;
}

auto Player::getIsActive() const -> bool {
  return m_bIsActive;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Player::getIsHuman() const -> bool {
  return m_bIsHuman;
}

auto Player::getName() const -> QString {
  return m_sName;
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::setCanMove(const quint8 nCanMove) {
  m_nCanMove = nCanMove;
}

auto Player::getCanMove() const -> quint8 {
  return m_nCanMove;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::setStonesLeft(const quint8 nStones) {
  if (nStones <= m_nMaxStones) {
    m_nStonesLeft = nStones;
  } else {
    m_nStonesLeft = m_nMaxStones;
    qWarning() << "Stones > MaxStones!" << nStones << ">" << m_nMaxStones;
    QMessageBox::warning(nullptr, QStringLiteral("Warning"),
                         QStringLiteral("Something went wrong!"));
  }
}

auto Player::getStonesLeft() const -> quint8 {
  return m_nStonesLeft;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::setWonTowers(const quint8 nWonTowers) {
  m_nWonTowers = nWonTowers;
}

auto Player::getWonTowers() const -> quint8 {
  return m_nWonTowers;
}
