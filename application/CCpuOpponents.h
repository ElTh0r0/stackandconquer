/**
 * \file CCpuOpponents.h
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
 * Class definition CPU opponents.
 */

#ifndef STACKANDCONQUER_CCPUOPPONENTS_H_
#define STACKANDCONQUER_CCPUOPPONENTS_H_

#include <QDir>

#include "./ICpuOpponent.h"

class CCpuOpponents : public QObject {
  Q_OBJECT

 public:
  CCpuOpponents(const QDir userDataDir);
  QStringList getCpuList();
  QObject* getCurrentCpu(qint8 nCpu);

 private:
  QList<ICpuOpponent *> m_listCPUs;
  QList<QObject *> m_listCpuObjects;
  QStringList m_sListAvailableCPUs;

  QDir m_userDataDir;
};

#endif  // STACKANDCONQUER_CCPUOPPONENTS_H_
