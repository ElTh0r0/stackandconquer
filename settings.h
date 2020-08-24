/**
 * \file settings.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2020 Thorsten Roth
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
 * Class definition for settings.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDialog>
#include <QMap>

class QComboBox;
class QLabel;
class QLineEdit;
class QSettings;

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
                      const quint8 nMaxPlayers, QWidget *pParent = nullptr);
    virtual ~Settings();

    auto getBoardFile() const -> QString;
    auto getPlayerCpuScript(const quint8 nPlayer) const -> QString;
    auto getPlayerColor(const quint8 nPlayer) const -> QString;
    auto getNumOfPlayers() const -> quint8;
    auto getMaxNumOfPlayers() const -> quint8;
    auto getStartPlayer() const -> quint8;
    auto getWinTowers() const -> quint8;
    auto getShowPossibleMoveTowers() const -> bool;
    auto getLanguage() -> QString;

    auto getBgColor() const -> QColor;
    auto getTextColor() const -> QColor;
    auto getTextHighlightColor() const -> QColor;
    auto getHighlightColor() const -> QColor;
    auto getHighlightBorderColor() const -> QColor;
    auto getSelectedColor() const -> QColor;
    auto getSelectedBorderColor() const -> QColor;
    auto getAnimateColor() const -> QColor;
    auto getAnimateBorderColor() const -> QColor;
    auto getBgBoardColor() const -> QColor;
    auto getGridBoardColor() const -> QColor;
    auto getNeighboursColor() const -> QColor;
    auto getNeighboursBorderColor() const -> QColor;

 public slots:
    void accept() override;
    void reject() override;
    void updateUiLang();

 signals:
    void newGame(const QString &s);
    void changeLang(const QString &sLang);

 protected:
    void showEvent(QShowEvent *pEvent) override;
    bool eventFilter(QObject *pObj, QEvent *pEvent) override;

 private slots:
    void changeNumOfPlayers();
    void changedSettings();

 private:
    void readSettings();
    auto readColor(const QString &sKey,
                   const QString &sFallback) const -> QColor;
    auto searchTranslations() const -> QStringList;
    void searchCpuScripts(const QString &userDataDir);
    void updateStartCombo();

    Ui::SettingsDialog *m_pUi;
    QSettings *m_pSettings;
    QList<QMap<QString, QString>> m_Players;
    QString m_sSharePath;
    QString m_sGuiLanguage;

    QList<QLabel*> m_listColorLbls;
    QList<QLabel*> m_listHumCpuLbls;
    QList<QLineEdit*> m_listColorEdit;
    QList<QComboBox*> m_listPlayerCombo;

    QStringList m_sListCPUs;
    int m_nNumOfPlayers{};
    int m_nStartPlayer{};
    int m_nWinTowers{};
    bool m_bShowPossibleMoveTowers{};
    bool m_bSettingChanged{};

    QColor m_bgColor;
    QColor m_txtColor;
    QColor m_txtHighColor;
    QColor m_highlightColor;
    QColor m_highlightBorderColor;
    QColor m_selectedColor;
    QColor m_selectedBorderColor;
    QColor m_animateColor;
    QColor m_animateBorderColor;
    QColor m_bgBoardColor;
    QColor m_gridBoardColor;
    QColor m_neighboursColor;
    QColor m_neighboursBorderColor;

    const quint8 m_nMaxPlayers;
    const QStringList m_DefaultPlayerColors;
};

#endif  // SETTINGS_H_
