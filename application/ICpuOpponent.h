/**
 * \file ICpuOpponent.h
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
 * CPU opponent interface.
 */

#ifndef STACKANDCONQUER_ICPUOPPONENT_H_
#define STACKANDCONQUER_ICPUOPPONENT_H_

#include <QtPlugin>
#include <QPoint>

class ICpuOpponent {
 public:
  virtual ~ICpuOpponent() {}

  // ALL FUNCTIONS PURE VIRTUAL !!!
  virtual void initCPU() = 0;
  virtual QString getCpuName() const = 0;

 signals:
  virtual void setStone(QPoint field) = 0;
  virtual void moveTower(QPoint tower, QPoint moveTo, quint8 nStones) = 0;

 public slots:
  virtual void makeMove(QList<QList<QList<quint8> > > board,
                        bool bStonesLeft) = 0;
};

Q_DECLARE_INTERFACE(ICpuOpponent, "StackAndConquer.CpuInterface")

#endif  // STACKANDCONQUER_ICPUOPPONENT_H_
