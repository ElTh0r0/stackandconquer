/**
 * \file player.h
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
 * Class definition for player.
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QString>

class OpponentJS;

/**
 * \class Player
 * \brief Player class.
 */
class Player : public QObject {
  Q_OBJECT

 public:
    Player(bool bActive, const quint8 nID, const QString &sName,
           const quint8 nMaxStones,
           const QString &sCpuScript = QLatin1String(""),
           QObject *parent = nullptr);
    ~Player();

    auto initCPU(const QPoint BoadDimensions, const quint8 nMaxTowerHeight,
                 const QString &sOut, const QString &sPad) -> bool;
    void setActive(const bool bActive);
    auto isActive() const -> bool;
    auto isHuman() const -> bool;
    auto getName() const -> QString;
    auto getID() const -> QString;
    void setStonesLeft(const quint8 nStones);
    auto getStonesLeft() const -> quint8;
    void setWonTowers(const quint8 nWonTowers);
    auto getWonTowers() const -> quint8;
    void setLegalMoves(const QJsonDocument &legalMoves);
    auto getLegalMoves() const -> QJsonDocument;
    auto canMove() const -> bool;
    void callCpu(const QJsonArray &board, const QJsonDocument &legalMoves);

 signals:
    void actionCPU(QJsonArray move);
    void scriptError();

 private:
    const quint8 m_nID;
    OpponentJS *m_pJsCpu;
    bool m_bIsActive;
    QString m_sName;
    const QString m_sCpuScript;
    const quint8 m_nMaxStones;
    quint8 m_nStonesLeft;
    quint8 m_nWonTowers;
    QJsonDocument m_LegalMoves;
};

#endif  // PLAYER_H_
