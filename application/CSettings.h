/**
 * \file CSettings.h
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
 * Class definition for settings.
 */

#ifndef STACKANDCONQUER_CSETTINGS_H_
#define STACKANDCONQUER_CSETTINGS_H_

#include <QDialog>
#include <QSettings>

#include "./CSettings.h"

namespace Ui {
class CSettingsDialog;
}

/**
 * \class CSettings
 * \brief Settings dialog.
 */
class CSettings : public QDialog {
  Q_OBJECT

 public:
  explicit CSettings(const QString &sSharePath, const QStringList slistCPUs,
                     QWidget *pParent = 0);
  virtual ~CSettings();

  QString getNameP1();
  QString getNameP2();
  QString getP2HumanCpu();
  quint8 getStartPlayer();
  quint8 getWinTowers();
  bool getShowPossibleMoveTowers();
  QList<quint8> getMouseControls() const;

  QColor getBgColor() const;
  QColor getHighlightColor() const;
  QColor getHighlightBorderColor() const;
  QColor getSelectedColor() const;
  QColor getSelectedBorderColor() const;
  QColor getAnimateColor() const;
  QColor getAnimateBorderColor() const;
  QColor getBgBoardColor() const;
  QColor getOutlineBoardColor() const;
  QColor getGridBoardColor() const;
  QColor GetNeighboursColor() const;
  QColor GetNeighboursBorderColor() const;

 public slots:
  void accept();
  void reject();

 signals:
  void newGame();

 private:
  void readSettings();
  QColor readColor(const QString sKey, const QString sFallback);

  QWidget *m_pParent;
  Ui::CSettingsDialog *m_pUi;
  QSettings *m_pSettings;

  QString m_sGuiLanguage;
  QString m_sNameP1;
  QString m_sNameP2;
  QString m_sP2HumanCpu;
  int m_nStartPlayer;
  int m_nWinTowers;
  bool m_bShowPossibleMoveTowers;

  QStringList m_sListMouseButtons;
  QList<quint8> m_listMouseButtons;
  QList<quint8> m_listMouseControls;

  QColor m_bgColor;
  QColor m_highlightColor;
  QColor m_highlightBorderColor;
  QColor m_selectedColor;
  QColor m_selectedBorderColor;
  QColor m_animateColor;
  QColor m_animateBorderColor;
  QColor m_bgBoardColor;
  QColor m_outlineBoardColor;
  QColor m_gridBoardColor;
  QColor m_neighboursColor;
  QColor m_neighboursBorderColor;
};

#endif  // STACKANDCONQUER_CSETTINGS_H_
