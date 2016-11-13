/**
 * \file CCpuOpponents.cpp
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
 * CPU opponents
 */

#include <QApplication>
#include <QDebug>
#include <QPluginLoader>

#include "./CCpuOpponents.h"

CCpuOpponents::CCpuOpponents(const QDir userDataDir)
  : m_userDataDir(userDataDir) {
  qDebug() << "Calling" << Q_FUNC_INFO;

  QList<QDir> listCPUsDir;
  // CPUs in user folder
  QDir cpuDir = m_userDataDir;
  if (cpuDir.cd("cpu")) {
    listCPUsDir << cpuDir;
  }
  // CPUs in app folder (Windows and debugging)
  cpuDir = qApp->applicationDirPath();
  if (cpuDir.cd("cpu")) {
    listCPUsDir << cpuDir;
  }
  // CPUs in standard installation folder (Linux)
  cpuDir = qApp->applicationDirPath() + "/../lib/"
           + qApp->applicationName().toLower();
  if (cpuDir.cd("cpu")) {
    listCPUsDir << cpuDir;
  }

  // Look for available CPUs
  foreach (QDir dir, listCPUsDir) {
    qDebug() << "CPU folder:" << dir.absolutePath();

    foreach (QString sFile, dir.entryList(QDir::Files)) {
      qDebug() << "CPU file:" << sFile;
      QPluginLoader loader(dir.absoluteFilePath(sFile));
      QObject *pCPU = loader.instance();
      if (pCPU) {
        ICpuOpponent *piCpu = qobject_cast<ICpuOpponent *>(pCPU);

        if (piCpu) {
          // Check for duplicates
          if (m_sListAvailableCPUs.contains(piCpu->getCpuName())) {
            continue;
          }
          piCpu->initCPU();
          m_sListAvailableCPUs << piCpu->getCpuName();
          m_listCPUs << piCpu;
          m_listCpuObjects << pCPU;
        }
      }
    }
  }
  if (0 == m_listCPUs.size()) {
    qWarning() << "No CPU opponents found!";
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QStringList CCpuOpponents::getCpuList() {
  return m_sListAvailableCPUs;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QObject* CCpuOpponents::getCurrentCpu(qint8 nCpu) {
  if (nCpu < 0 || nCpu > m_listCpuObjects.size() - 1) {
    return NULL;
  } else {
    return m_listCpuObjects[nCpu];
  }
}
