/**
 * \file player.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2022 Thorsten Roth
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
 * Player.
 */

#include "./player.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "./opponentjs.h"

Player::Player(const quint8 nID, const quint8 nMaxStones,
               const QString &sCpuScript, QObject *pParent)
    : m_nID(nID),
      m_pJsCpu(nullptr),
      m_sName(tr("Player") + " " + QString::number(m_nID)),
      m_sCpuScript(sCpuScript),
      m_nMaxStones(nMaxStones),
      m_nStonesLeft(nMaxStones),
      m_nWonTowers(0) {
  Q_UNUSED(pParent)
  if (!m_sCpuScript.isEmpty()) {
    QFileInfo fi(m_sCpuScript);
    m_sName += " (" + fi.baseName() + ")";
  }
  qDebug() << "Generating " + m_sName;
}

Player::~Player() = default;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Player::initCPU(const QPoint BoadDimensions, const quint8 nMaxTowerHeight,
                     const quint8 nNumOfPlayers, const QString &sOut,
                     const QString &sPad) -> bool {
  m_pJsCpu = new OpponentJS(m_nID, BoadDimensions, nMaxTowerHeight,
                            nNumOfPlayers, sOut, sPad);
  connect(m_pJsCpu, &OpponentJS::actionCPU, this, &Player::actionCPU);
  connect(m_pJsCpu, &OpponentJS::scriptError, this, &Player::scriptError);
  return m_pJsCpu->loadAndEvalCpuScript(m_sCpuScript);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::callCpu(const QJsonArray &board, const QJsonDocument &legalMoves,
                     const qint8 nDirection) {
  if (nullptr == m_pJsCpu) {
    qWarning() << "callCPU called for Human player P" + this->getID();
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
    return;
  }
  m_pJsCpu->callJsCpu(board, legalMoves, nDirection);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Player::isHuman() const -> bool { return m_sCpuScript.isEmpty(); }

auto Player::getName() const -> QString { return m_sName; }

auto Player::getCpuScript() const -> QString { return m_sCpuScript; }

auto Player::getID() const -> QString { return QString::number(m_nID); }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::setLegalMoves(const QJsonDocument &legalMoves) {
  m_LegalMoves = legalMoves;
}

auto Player::getLegalMoves() const -> QJsonDocument { return m_LegalMoves; }

auto Player::canMove() const -> bool { return !m_LegalMoves.isEmpty(); }

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

auto Player::getStonesLeft() const -> quint8 { return m_nStonesLeft; }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Player::setWonTowers(const quint8 nWonTowers) {
  m_nWonTowers = nWonTowers;
}

auto Player::getWonTowers() const -> quint8 { return m_nWonTowers; }
