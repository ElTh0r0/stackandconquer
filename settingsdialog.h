/**
 * \file settingsdialog.h
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
 * Class definition for settings dialog.
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <QDialog>

#include "./settings.h"

class QComboBox;
class QLabel;
class QLineEdit;

namespace Ui {
class SettingsDialog;
}

/**
 * \class SettingsDialog
 * \brief Settings dialog.
 */
class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget *pParent, const QString &userDataDir);
  virtual ~SettingsDialog();

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
  void readSettings();
  auto searchTranslations() const -> QStringList;
  void searchCpuScripts(const QString &sUserDataDir);
  auto getCpuStrength(const QString &sFilename) -> QString;
  void searchBoardStyles(const QString &sStyleDir);
  void fillStyleTable();
  void saveBoardStyle();
  auto getStyleColorFromTable(QColor color, const int nRow) -> QColor;
  void searchBoards(const QString &sUserDataDir);
  void updateStartPlayerCombo();

  Ui::SettingsDialog *m_pUi;
  Settings *m_pSettings;

  QList<QLabel *> m_listColorLbls;
  QList<QLabel *> m_listHumCpuLbls;
  QList<QLineEdit *> m_listColorEdit;
  QList<QComboBox *> m_listPlayerCombo;

  QStringList m_sListBoards;
  bool m_bSettingChanged{};
};

#endif  // SETTINGSDIALOG_H_
