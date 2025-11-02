// SPDX-FileCopyrightText: 2015-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STACKANDCONQUER_H_
#define STACKANDCONQUER_H_

#include <QDir>
#include <QMainWindow>
#include <QTranslator>

#include "./settings.h"
#include "./settingsdialog.h"

class QFrame;
class QGraphicsView;
class QGridLayout;
class QLabel;
class QSlider;

class Game;

namespace Ui {
class StackAndConquer;
}

class StackAndConquer : public QMainWindow {
  Q_OBJECT

 public:
  explicit StackAndConquer(const QDir &userDataPath,
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

  QTranslator m_translator;    // App translations
  QTranslator m_translatorQt;  // Qt translations
  QString m_sCurrLang;
  Settings *m_pSettings;
  SettingsDialog *m_pSettingsDialog;
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
