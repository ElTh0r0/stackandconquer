/**
 * \file generateboard.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2022 Thorsten Roth
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
 * along with StackAndConquer.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Generate json .stackboard file from txt input files.
 */

#ifndef GENERATEBOARD_H_
#define GENERATEBOARD_H_

#include <QFile>
#include <QStringList>

class QFileInfo;

class GenerateBoard {
 public:
  static void startGeneration(const QStringList &cmdArgs,
                              const QString &sBoardInExt,
                              const QString &sBoardOutExt,
                              const QString &sIN, const QString &sOUT);

 private:
  static auto checkCmdArgs(
      const QStringList &cmdArgs,
      const QString &sBoardInExt) -> QPair<QFileInfo, QFileInfo>;
  static void loopFiles(const QPair<QFileInfo, QFileInfo> &fiInOut,
                        const QString &sBoardInExt,
                        const QString &sBoardOutExt,
                        const QString &sIN, const QString &sOUT);
  static auto generateBoard(QFile *pInput, QFile *pOutput,
                            const QString &sIN, const QString &sOUT) -> bool;
};

#endif  // GENERATEBOARD_H_
