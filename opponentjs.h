// SPDX-FileCopyrightText: 2017-2024 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPPONENTJS_H_
#define OPPONENTJS_H_

#include <QJSValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QPoint>
#include <QVector>

class QJSEngine;

class OpponentJS : public QObject {
  Q_OBJECT

 public:
  explicit OpponentJS(QWidget *pParent, const quint8 nID,
                      const QPoint BoardDimensions,
                      const quint8 nHeightTowerWin, const quint8 nNumOfPlayers,
                      const QString &sOut, const QString &sPad,
                      QObject *pParentObj = nullptr);
  auto loadAndEvalCpuScript(const QString &sFilepath,
                            const QJsonArray &emptyBoard) -> bool;
  void callJsCpu(const QJsonArray &board, const QJsonDocument &legalMoves,
                 const QJsonArray &towersNeededToWin,
                 const QJsonArray &stonesLeft, const QJsonArray &lastMove);

  // Methods which are exposed to the CPU script
 public slots:
  void log(const QString &sMsg);
  quint8 getID();
  quint8 getNumOfPlayers();
  quint8 getHeightToWin();
  QJsonArray getTowersNeededToWin();
  QJsonArray getNumberOfStones();
  QJsonArray getLastMove();
  QString getLegalMoves();
  int getBoardDimensionX();
  int getBoardDimensionY();
  QString getOutside();
  QString getPadding();

 signals:
  void actionCPU(QJsonArray move);
  void scriptError();

 private:
  QWidget *m_pParent;
  const quint8 m_nID;
  const QPoint m_BoardDimensions;
  const quint8 m_nHeightTowerWin;
  const quint8 m_nNumOfPlayers;
  const QString m_sOut;
  const QString m_sPad;
  QJSEngine *m_jsEngine;
  QJSValue m_obj;
  QJsonArray m_TowersNeededToWin;
  QJsonArray m_StonesLeft;
  QJsonArray m_LastMove;
  QJsonDocument m_legalMoves;
};

#endif  // OPPONENTJS_H_
