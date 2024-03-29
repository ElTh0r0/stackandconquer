/**
 * \file stackandconquer.h
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
 * Class definition main application.
 */

#ifndef STACKANDCONQUER_H_
#define STACKANDCONQUER_H_

#include <QDir>
#include <QMainWindow>
#include <QTranslator>

class QFrame;
class QGraphicsView;
class QGridLayout;
class QLabel;
class QSlider;

class Game;
class Settings;

namespace Ui {
class StackAndConquer;
}

/**
 * \class StackAndConquer
 * \brief Main application definition (GUI, objects, etc.)
 */
class StackAndConquer : public QMainWindow {
  Q_OBJECT

 public:
  explicit StackAndConquer(const QDir &sharePath, const QDir &userDataPath,
                           const QString &sSaveExtension,
                           const QString &sBoardExtension, const QString &sIN,
                           const QString &sOUT,
                           const QStringList &sListArgs = QStringList(),
                           QWidget *pParent = nullptr);
  ~StackAndConquer();

 signals:
  void updateUiLang();
  void changeZoom();

 protected:
  void changeEvent(QEvent *pEvent) override;

 private slots:
  void startNewGame(const QString &sSavegame = QLatin1String(""));
  void loadGame();
  void saveGame();
  void setViewInteractive(const bool bEnabled);
  void highlightActivePlayer(const quint8 nActivePlayer,
                             const quint8 nPlayerWon = 0);
  void updateNames(const QStringList &sListName);
  void drawPlayerIcon(const quint8 nID);
  void zoomChanged(const int nNewGrid);
  void loadLanguage(const QString &sLang);
  void showRules();
  static void reportBug();
  void showInfoBox();

 private:
  void checkCmdArgs(const QStringList &sListArgs = QStringList());
  static auto switchTranslator(QTranslator *translator, const QString &sFile,
                               const QString &sPath = QString()) -> bool;
  void setupMenu();
  void setupGraphView();
  void resizeEvent(QResizeEvent *pEvent) override;
  void recolor();

  Ui::StackAndConquer *m_pUi;
  const QDir m_userDataDir;
  const QString m_sSharePath;
  const QString m_sSaveExtension;
  const QString m_sIN;
  const QString m_sOUT;
  const quint8 m_nMaxPlayers;
  QTranslator m_translator;    // App translations
  QTranslator m_translatorQt;  // Qt translations
  QString m_sCurrLang;
  Settings *m_pSettings;
  QGraphicsView *m_pGraphView{};
  Game *m_pGame;

  QFrame *m_pFrame{};
  QGridLayout *m_pLayout{};
  QList<QLabel *> m_pLblsPlayerName;
  QList<QLabel *> m_pLblsStoneIcon;
  QList<QLabel *> m_pLblsStonesLeft;
  QList<QLabel *> m_pLblsWinIcon;
  QList<QLabel *> m_pLblsWon;
  QSlider *m_pZoomSlider;
  const QSize m_DefaultSize;
};

#endif  // STACKANDCONQUER_H_
