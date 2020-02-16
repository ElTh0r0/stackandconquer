/**
 * \file opponentjs.h
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
 * Interface to CPU script JS engine.
 */

#ifndef OPPONENTJS_H_
#define OPPONENTJS_H_

#include <QObject>
#include <QPoint>
#include <QJSEngine>

class OpponentJS : public QObject {
  Q_OBJECT

 public:
    explicit OpponentJS(const quint8 nID,
                        const QPoint NumOfFields,
                        const quint8 nHeightTowerWin,
                        QObject *parent = nullptr);
    auto loadAndEvalCpuScript(const QString &sFilepath) -> bool;

 public slots:
    void makeMoveCpu(const QList<QList<QList<quint8> > > &board,
                     const quint8 nPossibleMove);
    static void log(const QString &sMsg);

 signals:
    void setStone(QPoint field);
    void moveTower(QPoint tower, QPoint moveTo, quint8 nStones);
    void scriptError();

 private:
    auto convertBoardToJSON(
        const QList<QList<QList<quint8> > > &board) -> QJsonDocument;
    static auto evalMoveReturn(const QString &sReturn) -> QList<QPoint>;

    const quint8 m_nID;
    const QPoint m_NumOfFields;
    const quint8 m_nHeightTowerWin;
    QJSEngine *m_jsEngine;
    QJSValue m_obj;
    QList<QList<QList<quint8> > > m_board;
};

#endif  // OPPONENTJS_H_
