/**
 * \file settings.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-present Thorsten Roth
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
  explicit Settings(QWidget *pParent, const QString &sSharePath,
                    const QString &userDataDir, const QString &sBoardExtension,
                    const quint8 nMaxPlayers);
  virtual ~Settings();

  auto getBoardFile() -> QString;
  auto getPlayerCpuScript(const quint8 nPlayer) const -> QString;
  auto getPlayerColor(const quint8 nPlayer) const -> QString;
  auto getNumOfPlayers() const -> quint8;
  auto getMaxNumOfPlayers() const -> quint8;
  auto getStartPlayer() const -> quint8;
  auto getTowersToWin() const -> quint8;
  auto getShowPossibleMoveTowers() const -> bool;
  auto getLanguage() -> QString;

  auto getGridSize() const -> quint16;
  void setGridSize(const quint16 nNewGrid);
  auto getDefaultGrid() const -> qreal;
  auto getMaxGrid() const -> quint16;

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
  void changedStyle(int nIndex);
  void clickedStyleCell(int nRow, int nCol);

 private:
  void copyDefaultStyles();
  void readSettings();
  auto readColor(QSettings *pSet, const QString &sKey,
                 const QString &sFallback) const -> QColor;
  auto searchTranslations() const -> QStringList;
  void searchCpuScripts(const QString &sUserDataDir);
  auto getCpuStrength(const QString &sFilename) -> QString;
  void searchBoardStyles(const QString &sStyleDir);
  void loadBoardStyle(const QString &sStyleFile);
  void readStyle_SetTable(QColor &color, QSettings *pSet, const int nRow,
                          const QString &sKey, const QString &sFallback);
  void saveBoardStyle(const QString &sStyleFile);
  void saveColor(QColor &color, QSettings *pSet, const int nRow,
                 const QString &sKey);
  void searchBoards(const QString &sUserDataDir);
  void updateStartCombo();

  Ui::SettingsDialog *m_pUi;
  QSettings *m_pSettings;
  QList<QMap<QString, QString>> m_Players;
  const QString m_sSharePath;
  const QString m_sBoardExtension;
  const QString m_sCpuExtension;
  QString m_sGuiLanguage;

  QList<QLabel *> m_listColorLbls;
  QList<QLabel *> m_listHumCpuLbls;
  QList<QLineEdit *> m_listColorEdit;
  QList<QComboBox *> m_listPlayerCombo;

  QStringList m_sListCPUs;
  QStringList m_sListBoards;
  QString m_sBoard;
  int m_nNumOfPlayers{};
  int m_nStartPlayer{};
  int m_nTowersToWin{};
  bool m_bShowPossibleMoveTowers{};
  bool m_bSettingChanged{};

  QString m_sBoardStyleFile;
  QString m_sExt;
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

  quint16 m_nGridSize;
  const qreal m_nDefaultGrid;
  const quint16 m_nMaxGrid;
  const quint8 m_nMaxPlayers;
  const QStringList m_DefaultPlayerColors;
};

#endif  // SETTINGS_H_
