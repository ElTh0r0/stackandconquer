/**
 * \file settings.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2018 Thorsten Roth <elthoro@gmx.de>
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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

/**
 * \class Settings
 * \brief Settings dialog.
 */
class Settings : public QDialog {
  Q_OBJECT

  public:
    explicit Settings(const QString &sSharePath, const QString &userDataDir,
                      QWidget *pParent = 0);
    virtual ~Settings();

    QString getNameP1() const;
    QString getNameP2() const;
    QString getP1HumanCpu() const;
    QString getP2HumanCpu() const;
    quint8 getStartPlayer() const;
    quint8 getWinTowers() const;
    bool getShowPossibleMoveTowers() const;
    QString getLanguage();

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
    void updateUiLang();

  signals:
    void newGame();
    void changeLang(const QString &sLang);

  private:
    void readSettings();
    QColor readColor(const QString sKey, const QString sFallback) const;
    QStringList searchTranslations() const;
    void searchCpuScripts(const QString &userDataDir);

    QWidget *m_pParent;
    Ui::SettingsDialog *m_pUi;
    QSettings *m_pSettings;

    QString m_sSharePath;
    QString m_sGuiLanguage;
    QString m_sNameP1;
    QString m_sNameP2;
    QString m_sP1HumanCpu;
    QString m_sP2HumanCpu;
    QStringList m_sListCPUs;
    int m_nStartPlayer;
    int m_nWinTowers;
    bool m_bShowPossibleMoveTowers;

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

#endif  // SETTINGS_H_
