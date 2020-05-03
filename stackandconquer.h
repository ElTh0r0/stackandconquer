/**
 * \file stackandconquer.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2020 Thorsten Roth <elthoro@gmx.de>
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
 * Class definition main application.
 */

#ifndef STACKANDCONQUER_H_
#define STACKANDCONQUER_H_

#include <QtCore>
#include <QFormLayout>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QtGui>
#include <QLabel>
#include <QMainWindow>

#include "./game.h"
#include "./settings.h"

namespace Ui {
class StackAndConquer;
}

/**
 * \class StackAndConquer
 * \brief Main application definition (gui, objects, etc.)
 */
class StackAndConquer : public QMainWindow {
  Q_OBJECT

 public:
    explicit StackAndConquer(const QDir &sharePath,
                             const QDir &userDataPath,
                             const QStringList &sListArgs = QStringList(),
                             QWidget *pParent = nullptr);
    ~StackAndConquer();

 signals:
    void updateUiLang();

 protected:
    void changeEvent(QEvent *pEvent);

 private slots:
    void startNewGame(const QStringList &sListArgs = QStringList());
    void loadGame();
    void saveGame();
    void setViewInteractive(const bool bEnabled);
    void highlightActivePlayer(const bool bPlayer1,
                               const bool bP1Won = false,
                               const bool bP2Won = false);
    void loadLanguage(const QString &sLang);
    void showRules();
    static void reportBug();
    void showInfoBox();

 private:
    void checkCmdArgs(const QStringList &sListArgs = QStringList());
    static auto switchTranslator(QTranslator *translator,
                                 const QString &sFile,
                                 const QString &sPath = QString()) -> bool;
    void setupMenu();
    void setupGraphView();
    void resizeEvent(QResizeEvent *pEvent);

    Ui::StackAndConquer *m_pUi;
    const QDir m_userDataDir;
    const QString m_sSharePath;
    QTranslator m_translator;  // App translations
    QTranslator m_translatorQt;  // Qt translations
    QString m_sCurrLang;
    Settings *m_pSettings;
    QGraphicsView *m_pGraphView{};
    Game *m_pGame;

    QFrame *m_pFrame{};
    QGridLayout *m_pLayout{};
    QLabel *m_plblPlayer1{};
    QLabel *m_plblPlayer2{};
    QLabel *m_plblIconStones1{};
    QLabel *m_plblIconWin1{};
    QLabel *m_plblIconStones2{};
    QLabel *m_plblIconWin2{};
    QLabel *m_plblP1StonesLeft{};
    QLabel *m_plblP2StonesLeft{};
    QLabel *m_plblP1Won{};
    QLabel *m_plblP2Won{};
};

#endif  // STACKANDCONQUER_H_
