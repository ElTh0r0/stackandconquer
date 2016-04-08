/**
 * \file CStackAndConquer.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2016 Thorsten Roth <elthoro@gmx.de>
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
 * Class definition main application.
 */

#ifndef STACKANDCONQUER_CSTACKANDCONQUER_H_
#define STACKANDCONQUER_CSTACKANDCONQUER_H_

#include <QtCore>
#include <QFormLayout>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QtGui>
#include <QLabel>
#include <QMainWindow>

#include "./CBoard.h"
#include "./CPlayer.h"

namespace Ui {
    class CStackAndConquer;
}

/**
 * \class CStackAndConquer
 * \brief Main application definition (gui, objects, etc.)
 */
class CStackAndConquer : public QMainWindow {
    Q_OBJECT

  public:
    explicit CStackAndConquer(QWidget *pParent = 0);
    ~CStackAndConquer();

  protected:
    void closeEvent(QCloseEvent *pEvent);

  private slots:
    void startNewGame();
    void setStone(QPoint field);
    void moveTower(QPoint tower, QPoint moveTo);
    void reportBug();
    void showInfoBox();

  private:
    void setupMenu();

    Ui::CStackAndConquer *m_pUi;
    QGraphicsView *m_pGraphView;

    CBoard *m_pBoard;
    CPlayer *m_pPlayer1;
    CPlayer *m_pPlayer2;

    const quint8 m_nMaxTowerHeight;
    const quint8 m_nNumToWin;
    const quint8 m_nMaxStones;
    const quint16 m_nGridSize;

    QLabel *m_plblPlayer1;
    QLabel *m_plblPlayer2;
    QLabel *m_plblPlayer1StonesLeft;
    QLabel *m_plblPlayer2StonesLeft;
    QLabel *m_plblPlayer1Won;
    QLabel *m_plblPlayer2Won;
    QFrame *m_pFrame1;
    QFrame *m_pFrame2;
    QFormLayout *m_pLayout1;
    QFormLayout *m_pLayout2;

    void checkTowerWin(QPoint field);
    void returnStones(QPoint field);
    void updatePlayers(bool bInitial = false);
};

#endif  // STACKANDCONQUER_CSTACKANDCONQUER_H_
