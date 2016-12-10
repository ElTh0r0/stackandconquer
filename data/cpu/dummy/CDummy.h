/**
 * \file CDummy.h
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
 * Class definition for dummy CPU.
 */

#ifndef STACKANDCONQUER_CDUMMY_H_
#define STACKANDCONQUER_CDUMMY_H_

#include <QtPlugin>

#include "../../application/ICpuOpponent.h"

/**
 * \class CDummy
 * \brief Dummy CPU opponent
 */
class CDummy : public QObject, ICpuOpponent {
  Q_OBJECT
  Q_INTERFACES(ICpuOpponent)

#if QT_VERSION >= 0x050000
  Q_PLUGIN_METADATA(IID "StackAndConquer.cpudummy")
#endif

 public:
  void initCPU();
  QString getCpuName() const;

 signals:
  void setStone(QPoint field);
  void moveTower(QPoint tower, QPoint moveTo, quint8 nStones);

 public slots:
  void makeMove(QList<QList<QList<quint8> > > board, bool bStonesLeft);

 private:
  void setRandom();

  QList<QList<QList<quint8> > > m_board;
};

#endif  // STACKANDCONQUER_CDUMMY_H_
