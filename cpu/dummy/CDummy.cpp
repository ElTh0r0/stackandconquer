/**
 * \file CDummy.cpp
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
 * Dummy CPU opponent.
 */

#include <QDebug>

#include "./CDummy.h"

void CDummy::initCPU() {
  qDebug() << "initCPU()" << PLUGIN_NAME << PLUGIN_VERSION <<
              "-" << this->getCpuName();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QString CDummy::getCpuName() const {
  return "Dummy CPU";
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CDummy::makeMove(QList<QList<QList<quint8> > > board, bool bStonesLeft) {
  m_board = board;

  if (bStonesLeft) {
    this->setRandom();
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CDummy::setRandom() {
  int nRandX = 0;
  int nRandY = 0;

  do {
    nRandX = qrand() % 5;
    nRandY = qrand() % 5;
  } while (0 != m_board[nRandX][nRandY].size());

  emit this->setStone(QPoint(nRandX, nRandY));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(cpudummy, CDummy)
#endif
