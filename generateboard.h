// SPDX-FileCopyrightText: 2022 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERATEBOARD_H_
#define GENERATEBOARD_H_

#include <QFile>
#include <QStringList>

class QFileInfo;

class GenerateBoard {
 public:
  static void startGeneration(const QStringList &cmdArgs,
                              const QString &sBoardInExt,
                              const QString &sBoardOutExt, const QString &sIN,
                              const QString &sOUT);

 private:
  static auto checkCmdArgs(const QStringList &cmdArgs,
                           const QString &sBoardInExt)
      -> QPair<QFileInfo, QFileInfo>;
  static void loopFiles(const QPair<QFileInfo, QFileInfo> &fiInOut,
                        const QString &sBoardInExt, const QString &sBoardOutExt,
                        const QString &sIN, const QString &sOUT);
  static auto generateBoard(QFile *pInput, QFile *pOutput, const QString &sIN,
                            const QString &sOUT) -> bool;
};

#endif  // GENERATEBOARD_H_
