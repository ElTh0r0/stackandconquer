/**
 * \file CStackAndConquer.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2017 Thorsten Roth <elthoro@gmx.de>
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
 * Main application generation (gui)
 */

#include <QApplication>
#include <QGridLayout>
#include <QMessageBox>
#include <QTextEdit>

#include "./CStackAndConquer.h"
#include "ui_CStackAndConquer.h"

CStackAndConquer::CStackAndConquer(const QDir &sharePath,
                                   const QDir &userDataPath,
                                   QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::CStackAndConquer),
    m_sSharePath(sharePath.absolutePath()),
    m_sCurrLang(""),
    m_pGame(NULL) {
  qDebug() << "Starting...";

  m_pUi->setupUi(this);
  this->setWindowTitle(qApp->applicationName());

  m_pSettings = new CSettings(m_sSharePath, userDataPath.absolutePath(), this);
  connect(m_pSettings, SIGNAL(newGame()),
          this, SLOT(startNewGame()));
  connect(m_pSettings, SIGNAL(changeLang(QString)),
          this, SLOT(loadLanguage(QString)));
  connect(this, SIGNAL(updateUiLang()),
          m_pSettings, SLOT(updateUiLang()));
  this->loadLanguage(m_pSettings->getLanguage());

  this->setupMenu();
  this->setupGraphView();

  // Seed random number generator
  QTime time = QTime::currentTime();
  qsrand((uint)time.msec());

  // Choose CPU script or load game from command line
  QString sCmdArg("");
  if (qApp->arguments().size() > 1) {
    if (qApp->arguments()[1].endsWith(".js", Qt::CaseInsensitive) ||
        qApp->arguments()[1].endsWith(".json", Qt::CaseInsensitive)) {
      if (QFile::exists(qApp->arguments()[1])) {
        sCmdArg = qApp->arguments()[1];
      } else {
        qWarning() << "Specified JS file not found:" << qApp->arguments()[1];
        QMessageBox::warning(this, trUtf8("Warning"),
                             trUtf8("Specified file not found:") + "\n" +
                             qApp->arguments()[1]);
      }
    }
  }

  this->startNewGame(sCmdArg);
}

CStackAndConquer::~CStackAndConquer() {
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setupMenu() {
  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, SIGNAL(triggered()),
          this, SLOT(startNewGame()));

  // TODO: Load / save game
  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  // connect(m_pUi->action_LoadGame, SIGNAL(triggered()),
  //         this, SLOT(loadGame()));
  m_pUi->action_LoadGame->setEnabled(false);

  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  // connect(m_pUi->action_SaveGame, SIGNAL(triggered()),
  //         this, SLOT(saveGame()));

  // Settings
  connect(m_pUi->action_Preferences, SIGNAL(triggered()),
          m_pSettings, SLOT(show()));

  // Exit game
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  connect(m_pUi->action_Quit, SIGNAL(triggered()),
          this, SLOT(close()));

  // Show rules
  connect(m_pUi->action_Rules, SIGNAL(triggered()),
          this, SLOT(showRules()));

  // Report bug
  connect(m_pUi->action_ReportBug, SIGNAL(triggered()),
          this, SLOT(reportBug()));

  // About
  connect(m_pUi->action_Info, SIGNAL(triggered()),
          this, SLOT(showInfoBox()));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setupGraphView() {
  m_pGraphView = new QGraphicsView(this);
  // Set mouse tracking to true, otherwise mouse move event
  // for the *scene* is only triggered on a mouse click!
  // QGraphicsView forwards the event to the scene.
  m_pGraphView->setMouseTracking(true);

  // TODO: Scalable window/board/stones
  // Transform coordinate system to "isometric" view
  QTransform transfISO;
  transfISO = transfISO.scale(1.0, 0.5).rotate(45);
  m_pGraphView->setTransform(transfISO);
  this->setCentralWidget(m_pGraphView);

  m_pFrame = new QFrame(m_pGraphView);
  m_pLayout = new QGridLayout;
  m_pLayout->setVerticalSpacing(0);
  m_plblPlayer1 = new QLabel(m_pSettings->getNameP1());
  m_plblP1StonesLeft = new QLabel("99");
  m_plblP1Won = new QLabel("0");
  m_plblPlayer2 = new QLabel(m_pSettings->getNameP2());
  m_plblP2StonesLeft = new QLabel("99");
  m_plblP2StonesLeft->setAlignment(Qt::AlignRight);
  m_plblP2Won = new QLabel("0");
  m_plblP2Won->setAlignment(Qt::AlignRight);

  QPixmap iconStones(":/images/stones.png");
  m_plblIconStones = new QLabel();
  m_plblIconStones->setPixmap(iconStones);
  m_plblIconStones->setAlignment(Qt::AlignCenter);
  QPixmap iconWin(":/images/win.png");
  m_plblIconWin = new QLabel();
  m_plblIconWin->setPixmap(iconWin);
  m_plblIconWin->setAlignment(Qt::AlignCenter);

  // addWidget(*widget, row, column, rowspan, colspan)
  m_pLayout->addWidget(m_plblPlayer1, 0, 0, 1, 1);
  m_pLayout->addWidget(m_plblPlayer2, 0, 2, 1, 1);
  m_pLayout->addWidget(m_plblP1StonesLeft, 1, 0, 1, 1);
  m_pLayout->addWidget(m_plblIconStones, 1, 1, 1, 1);
  m_pLayout->addWidget(m_plblP2StonesLeft, 1, 2, 1, 1);
  m_pLayout->addWidget(m_plblP1Won, 2, 0, 1, 1);
  m_pLayout->addWidget(m_plblIconWin, 2, 1, 1, 1);
  m_pLayout->addWidget(m_plblP2Won, 2, 2, 1, 1);
  m_pFrame->setLayout(m_pLayout);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::startNewGame(const QString sCmdArg) {
  qDebug() << Q_FUNC_INFO;

  if (NULL != m_pGame) {
    delete m_pGame;
  }

  m_pGame = new CGame(m_pSettings, sCmdArg);

  connect(m_pGame, SIGNAL(updateNameP1(QString)),
          m_plblPlayer1, SLOT(setText(QString)));
  connect(m_pGame, SIGNAL(updateNameP2(QString)),
          m_plblPlayer2, SLOT(setText(QString)));

  connect(m_pGame, SIGNAL(updateStonesP1(QString)),
          m_plblP1StonesLeft, SLOT(setText(QString)));
  connect(m_pGame, SIGNAL(updateStonesP2(QString)),
          m_plblP2StonesLeft, SLOT(setText(QString)));

  connect(m_pGame, SIGNAL(updateWonP1(QString)),
          m_plblP1Won, SLOT(setText(QString)));
  connect(m_pGame, SIGNAL(updateWonP2(QString)),
          m_plblP2Won, SLOT(setText(QString)));

  connect(m_pGame, SIGNAL(setInteractive(bool)),
          this, SLOT(setViewInteractive(bool)));
  connect(m_pGame, SIGNAL(highlightActivePlayer(bool, bool, bool)),
          this, SLOT(highlightActivePlayer(bool, bool, bool)));

  m_pGraphView->setScene(m_pGame->getScene());
  m_pGraphView->updateSceneRect(m_pGame->getSceneRect());
  m_pGraphView->setInteractive(true);

  if ("Human" != m_pSettings->getP1HumanCpu() ||
      "Human" != m_pSettings->getP2HumanCpu()) {
    if (!m_pGame->initCpu()) {
      m_pGraphView->setInteractive(false);
      QMessageBox::warning(this, trUtf8("Warning"),
                           trUtf8("An error occured during CPU initialization."));
      return;
    }
  }
  m_pGame->updatePlayers(true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setViewInteractive(const bool bEnabled) {
  m_pGraphView->setInteractive(bEnabled);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::highlightActivePlayer(const bool bPlayer1,
                                             const bool bP1Won,
                                             const bool bP2Won) {
  if (bP1Won) {
    m_pUi->statusBar->showMessage(
          trUtf8("%1 won the game!").arg(m_plblPlayer1->text()));
    return;
  } else if (bP2Won) {
    m_pUi->statusBar->showMessage(
          trUtf8("%1 won the game!").arg(m_plblPlayer2->text()));
    return;
  }

  if (bPlayer1) {
    m_plblPlayer1->setStyleSheet("color: #FF0000");
    m_plblPlayer2->setStyleSheet("color: #000000");
    m_pUi->statusBar->showMessage(
          trUtf8("%1's turn").arg(m_plblPlayer1->text()));
  } else {
    m_plblPlayer1->setStyleSheet("color: #000000");
    m_plblPlayer2->setStyleSheet("color: #FF0000");
    m_pUi->statusBar->showMessage(
          trUtf8("%1's turn").arg(m_plblPlayer2->text()));
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CStackAndConquer::loadLanguage(const QString &sLang) {
  if (m_sCurrLang != sLang) {
    m_sCurrLang = sLang;
    if (!this->switchTranslator(m_translatorQt, "qt_" + sLang,
                                QLibraryInfo::location(
                                  QLibraryInfo::TranslationsPath))) {
      this->switchTranslator(m_translatorQt, "qt_" + sLang,
                             m_sSharePath + "/lang");
    }
    this->switchTranslator(m_translator,
                           qApp->applicationName().toLower() + "_" + sLang,
                           m_sSharePath + "/lang");
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CStackAndConquer::switchTranslator(QTranslator &translator,
                                        const QString &sFile,
                                        const QString &sPath) {
  qApp->removeTranslator(&translator);
  if (translator.load(sFile, sPath)) {
    qApp->installTranslator(&translator);
  } else {
    if (!sFile.endsWith("_en")) {  // EN is build in translation -> no file
      qWarning() << "Could not find translation" << sFile << "in" << sPath;
    }
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::showRules() {
  QDialog* dialog = new QDialog(this, this->windowFlags()
                                & ~Qt::WindowContextHelpButtonHint);
  QGridLayout* layout = new QGridLayout(dialog);
  dialog->setWindowTitle(trUtf8("Rules"));
  dialog->setMinimumSize(700, 450);

  QTextEdit* textEdit = new QTextEdit;
  textEdit->setReadOnly(true);
  QLabel* credits = new QLabel;
  credits->setOpenExternalLinks(true);

  layout->setMargin(2);
  layout->setSpacing(0);
  layout->addWidget(textEdit);
  layout->addWidget(credits);

  QString s("");
  s += "<h2>" + trUtf8("Objective") + "</h2>" +
       "<p>" + trUtf8("Players try to build stacks, at least five pieces high, with a piece of their own color on top.") + "</p>";
  s += "<h2>" + trUtf8("Play") + "</h2>" +
       "<p>" + trUtf8("<strong>Note:</strong> In the following, a single piece is also be referred as a \"stack\".") + "</p>" +
       "<p>" + trUtf8("In each turn players have two options, one of which they must choose:") + "</p>" +
       "<ul><li>" + trUtf8("<strong>Enter</strong> a piece") + "</li><li>" + trUtf8("<strong>Move</strong> a stack") + "</li></ul>";
  s += "<h3>" + trUtf8("Enter a piece") + "</h3>" +
       "<p>" + trUtf8("A player places a new piece of his own color on any empty space on the board.") + "</p>";
  s += "<h3>" + trUtf8("Move a stack") + "</h3>" +
       "<p>" + trUtf8("A player chooses a stack and moves one or more pieces from there.") + "</p>" +
       "<ul><li>" + trUtf8("Pieces move <strong>orthogonally</strong> or <strong>diagonally</strong> in straight lines.") + "</li>"
       "<li>" + trUtf8("Pieces are always taken from the top of a stack.") + "</li>" +
       "<li>" + trUtf8("Stacks may be <strong>split</strong> at <strong>any</strong> level. Remaining pieces stay behind.") + "</li>" +
       "<li>" + trUtf8("Players may move pieces of <strong>any</strong>(!) color. It is possible to move single opponent piece or a stack with an opponent piece on top.") + "</li>" +
       "<li>" + trUtf8("It is not allowed to move such that the last move of the opponent is effectively taken back. Please note that such a move is often not available.") + "</li>" +
       "<li>" + trUtf8("Stacks may <strong>not</strong> cross occupied spaces.") + "</li>" +
       "<li>" + trUtf8("A move <strong>must end on another stack</strong>, moved pieces are placed on top of the target stack.") + "</li>" +
       "<li>" + trUtf8("Stacks may be <strong>any</strong> height.") + "</li></ul>";
  s += "<p>" + trUtf8("And, most important:") + "</p>" +
       "<ul><li>" + trUtf8("<strong>The height of the <u>target stack</u> determines the exact distance from where it can be reached.</strong>") + "</li></ul>" +
       "<p>" + trUtf8("<strong>Note:</strong>") + "</p>" +
       "<ul><li>" + trUtf8("The top piece does <strong>not</strong> determine ownership of a stack when it comes to move options.") + "</li>" +
       "<li>" + trUtf8("It is <strong>not</strong> the height of the moving stack that determines the length of a move. It is the height of the target stack that defines it.") + "</li></ul>";
  s += "<h3>" + trUtf8("Examples") + "</h3>";
  s += "<p></p><img src=\"" + m_sSharePath + "/example1.png\" alt=\"Example 1\" />";
  s += "<p>" + trUtf8("The stack at e4 may move three spaces onto the stack at b4 (because it is three pieces high). The stack at e1 cannot reach b4 because it is blocked by c3. Please note that b4 may <strong>not</strong> jump on e4!") + "</p>";
  s += "<p></p><img src=\"" + m_sSharePath + "/example2.png\" alt=\"Example 2\" />";
  s += "<p>" + trUtf8("The piece in the center (at c3) is within reach of all adjacent stacks because it is a stack of height one and each of the surrounding stacks is one step away.") + "</p>";
  s += "<h3>" + trUtf8("Pass") + "</h3>" +
       "<p>" + trUtf8("If a player cannot enter a new piece they must <strong>move</strong> a stack. If no move is available, the player must pass. The game ends in a draw if both players pass in sequence.") + "</p>";
  s += "<h2>" + trUtf8("End of the game") + "</h2>" +
       "<p>" + trUtf8("When a player builds a stack of height 5 (or higher), this stack is removed from the board, the pieces are put back to the reserve and the player who owned the top piece of said stack, scores <strong>1 point</strong>.") + "</p>" +
       "<p>" + trUtf8("For a standard game only <strong>1 point</strong> is needed to win, but players can agree upon any other number at the beginning of the game.") + "</p>";
  textEdit->setHtml(s);

  credits->setText("<p>" + trUtf8("These rules are licensed under Creative Commons <a href=\"https://creativecommons.org/licenses/by-nc/4.0/\">Attribution-Noncommercial 4.0 International</a> license.") +
                   "<br />Designer: Dieter Stein, <a href=\"https://spielstein.com/games/mixtour/rules\">spielstein.com</a></p>");

  dialog->show();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::reportBug() const {
  QDesktopServices::openUrl(QUrl("https://github.com/ElTh0r0/stackandconquer/issues"));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CStackAndConquer::showInfoBox() {
  QMessageBox::about(this, trUtf8("About"),
                     QString("<center>"
                             "<big><b>%1 %2</b></big><br/>"
                             "%3<br/>"
                             "<small>%4</small><br/><br/>"
                             "%5<br/>"
                             "%6<br/>"
                             "<small>%7</small>"
                             "</center><br/>"
                             "%8")
                     .arg(qApp->applicationName())
                     .arg(qApp->applicationVersion())
                     .arg(APP_DESC)
                     .arg(APP_COPY)
                     .arg("URL: <a href=\"https://github.com/ElTh0r0/stackandconquer\">"
                          "https://github.com/ElTh0r0/stackandconquer</a>")
                     .arg(trUtf8("License") +
                          ": "
                          "<a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                          "GNU General Public License Version 3</a>")
                     .arg(trUtf8("This application uses icons from "
                                 "<a href=\"http://tango.freedesktop.org\">"
                                 "Tango project</a>."))
                     .arg("<i>" + trUtf8("Translations") +
                          "</i><br />&nbsp;&nbsp;- German: ElThoro"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::changeEvent(QEvent *pEvent) {
  if (0 != pEvent) {
    if (QEvent::LanguageChange == pEvent->type()) {
      m_pUi->retranslateUi(this);
      emit updateUiLang();
    }
  }
  QMainWindow::changeEvent(pEvent);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// Close event (File -> Close or X)
void CStackAndConquer::closeEvent(QCloseEvent *pEvent) {
  pEvent->accept();
  /*
  int nRet = QMessageBox::question(this, trUtf8("Quit") + " - " +
                                   qApp->applicationName(),
                                   trUtf8("Do you really want to quit?"),
                                   QMessageBox::Yes | QMessageBox::No);

  if (QMessageBox::Yes == nRet) {
    pEvent->accept();
  } else {
    pEvent->ignore();
  }
  */
}
