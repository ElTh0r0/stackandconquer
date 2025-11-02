// SPDX-FileCopyrightText: 2015-2024 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLAYER_H_
#define PLAYER_H_

#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QString>

class OpponentJS;

class Player : public QObject {
  Q_OBJECT

 public:
  Player(QWidget *pParent, const quint8 nID, const quint8 nMaxStones,
         const QString &sCpuScript = QLatin1String(""),
         QObject *pParentObj = nullptr);
  ~Player();

  auto initCPU(const QJsonArray &emptyBoard, const QPoint BoardDimensions,
               const quint8 nMaxTowerHeight, const quint8 nNumOfPlayers,
               const QString &sOut, const QString &sPad) -> bool;
  auto isHuman() const -> bool;
  auto getName() const -> QString;
  auto getCpuScript() const -> QString;
  auto getID() const -> QString;
  void setStonesLeft(const quint8 nStones);
  auto getStonesLeft() const -> quint8;
  void setWonTowers(const quint8 nWonTowers);
  auto getWonTowers() const -> quint8;
  void setLegalMoves(const QJsonDocument &legalMoves);
  auto getLegalMoves() const -> QJsonDocument;
  auto canMove() const -> bool;
  void callCpu(const QJsonArray &board, const QJsonDocument &legalMoves,
               const QJsonArray &towersNeededToWin,
               const QJsonArray &stonesLeft, const QJsonArray &lastMove);

 signals:
  void actionCPU(QJsonArray move);
  void scriptError();

 private:
  QWidget *m_pParent;
  const quint8 m_nID;
  OpponentJS *m_pJsCpu;
  QString m_sName;
  const QString m_sCpuScript;
  const quint8 m_nMaxStones;
  quint8 m_nStonesLeft;
  quint8 m_nWonTowers;
  QJsonDocument m_LegalMoves;
};

#endif  // PLAYER_H_
