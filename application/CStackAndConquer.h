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

#include "./CCpuOpponents.h"
#include "./CGame.h"
#include "./CSettings.h"

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
  explicit CStackAndConquer(const QDir &sharePath,
                            const QDir &userDataPath,
                            QWidget *pParent = 0);
  ~CStackAndConquer();

 protected:
  void closeEvent(QCloseEvent *pEvent);

 private slots:
  void startNewGame();
  void setViewInteractive(bool bEnabled);
  void highlightActivePlayer(bool bPlayer1);
  void reportBug();
  void showInfoBox();

 private:
  void setupMenu();
  void setupGraphView();

  Ui::CStackAndConquer *m_pUi;
  CCpuOpponents *m_pCpu;
  QObject *m_piCpu;
  CSettings *m_pSettings;
  QGraphicsView *m_pGraphView;
  CGame *m_pGame;
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
};

#endif  // STACKANDCONQUER_CSTACKANDCONQUER_H_