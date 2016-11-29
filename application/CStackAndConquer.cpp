/**
 * \file CStackAndConquer.cpp
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
 * Main application generation (gui)
 */

#include <QApplication>
#include <QGridLayout>
#include <QMessageBox>

#include "./CStackAndConquer.h"
#include "ui_CStackAndConquer.h"

CStackAndConquer::CStackAndConquer(const QDir &sharePath,
                                   const QDir &userDataPath,
                                   QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::CStackAndConquer),
    m_sSharePath(sharePath.absolutePath()),
    m_sCurrLang(""),
    m_piCpu(NULL),
    m_pGame(NULL) {
  qDebug() << Q_FUNC_INFO;

  m_pUi->setupUi(this);
  this->setWindowTitle(qApp->applicationName());

  m_pCpu = new CCpuOpponents(userDataPath);
  m_pSettings = new CSettings(m_sSharePath,
                              m_pCpu->getCpuList(), this);
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

  this->startNewGame();
}

CStackAndConquer::~CStackAndConquer() {
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setupMenu() {
  qDebug() << Q_FUNC_INFO;

  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  m_pUi->action_NewGame->setIcon(QIcon::fromTheme("document-new"));
  connect(m_pUi->action_NewGame, SIGNAL(triggered()),
          this, SLOT(startNewGame()));

  // TODO: Load / save game
  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  m_pUi->action_LoadGame->setIcon(QIcon::fromTheme("document-open"));
  // connect(m_pUi->action_LoadGame, SIGNAL(triggered()),
  //         this, SLOT(loadGame()));
  m_pUi->action_LoadGame->setEnabled(false);

  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  m_pUi->action_SaveGame->setIcon(QIcon::fromTheme("document-save"));
  // connect(m_pUi->action_SaveGame, SIGNAL(triggered()),
  //         this, SLOT(saveGame()));

  // Settings
  m_pUi->action_Preferences->setIcon(QIcon::fromTheme("preferences-system"));
  connect(m_pUi->action_Preferences, SIGNAL(triggered()),
          m_pSettings, SLOT(show()));

  // Exit game
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  m_pUi->action_Quit->setIcon(QIcon::fromTheme("application-exit"));
  connect(m_pUi->action_Quit, SIGNAL(triggered()),
          this, SLOT(close()));

  // Report bug
  connect(m_pUi->action_ReportBug, SIGNAL(triggered()),
          this, SLOT(reportBug()));

  // About
  m_pUi->action_Info->setIcon(QIcon::fromTheme("help-about"));
  connect(m_pUi->action_Info, SIGNAL(triggered()),
          this, SLOT(showInfoBox()));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setupGraphView() {
  qDebug() << Q_FUNC_INFO;

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

  m_pFrame1 = new QFrame(m_pGraphView);
  m_pLayout1 = new QFormLayout;
  m_plblPlayer1 = new QLabel(m_pSettings->getNameP1());
  m_plblPlayer1StonesLeft = new QLabel(trUtf8("Stones left:") + " " +
                                       QString::number(99));
  m_plblPlayer1Won = new QLabel(trUtf8("Won:") + " 0");
  m_pLayout1->setVerticalSpacing(0);
  m_pLayout1->addRow(m_plblPlayer1);
  m_pLayout1->addRow(m_plblPlayer1StonesLeft);
  m_pLayout1->addRow(m_plblPlayer1Won);
  m_pFrame1->setLayout(m_pLayout1);

  m_pFrame2 = new QFrame(m_pGraphView);
  m_pLayout2 = new QFormLayout;
  m_plblPlayer2 = new QLabel(m_pSettings->getNameP2());
  m_plblPlayer2->setAlignment(Qt::AlignRight);
  m_plblPlayer2StonesLeft = new QLabel(QString::number(99));
  m_plblPlayer2Won = new QLabel("0");
  m_pLayout2->setVerticalSpacing(0);
  m_pLayout2->addRow(m_plblPlayer2);
  m_pLayout2->addRow(trUtf8("Stones left:"), m_plblPlayer2StonesLeft);
  m_pLayout2->addRow(trUtf8("Won:"), m_plblPlayer2Won);
  m_pFrame2->setLayout(m_pLayout2);
  m_pFrame2->move(this->width() - m_pLayout2->sizeHint().width(), 0);

}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::startNewGame() {
  qDebug() << Q_FUNC_INFO;

  if (NULL != m_pGame) {
    delete m_pGame;
  }

  m_piCpu = m_pCpu->getCurrentCpu(
              m_pCpu->getCpuList().indexOf(m_pSettings->getP2HumanCpu()));
  m_pGame = new CGame(m_pSettings, m_piCpu);

  connect(m_pGame, SIGNAL(updateNameP1(QString)),
          m_plblPlayer1, SLOT(setText(QString)));
  connect(m_pGame, SIGNAL(updateNameP2(QString)),
          m_plblPlayer2, SLOT(setText(QString)));

  connect(m_pGame, SIGNAL(updateStonesP1(QString)),
          m_plblPlayer1StonesLeft, SLOT(setText(QString)));
  connect(m_pGame, SIGNAL(updateStonesP2(QString)),
          m_plblPlayer2StonesLeft, SLOT(setText(QString)));

  connect(m_pGame, SIGNAL(updateWonP1(QString)),
          m_plblPlayer1Won, SLOT(setText(QString)));
  connect(m_pGame, SIGNAL(updateWonP2(QString)),
          m_plblPlayer2Won, SLOT(setText(QString)));

  connect(m_pGame, SIGNAL(setInteractive(bool)),
          this, SLOT(setViewInteractive(bool)));
  connect(m_pGame, SIGNAL(highlightActivePlayer(bool)),
          this, SLOT(highlightActivePlayer(bool)));

  m_pGraphView->setScene(m_pGame->getScene());
  m_pGraphView->updateSceneRect(m_pGame->getSceneRect());
  m_pGraphView->setInteractive(true);
  m_pGame->updatePlayers(true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setViewInteractive(bool bEnabled) {
  m_pGraphView->setInteractive(bEnabled);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::highlightActivePlayer(bool bPlayer1) {
  if (bPlayer1) {
    m_plblPlayer1->setStyleSheet("color: #FF0000");
    m_plblPlayer2->setStyleSheet("color: #000000");
  } else {
    m_plblPlayer1->setStyleSheet("color: #000000");
    m_plblPlayer2->setStyleSheet("color: #FF0000");
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

void CStackAndConquer::reportBug() {
  QDesktopServices::openUrl(QUrl("https://bugs.launchpad.net/stackandconquer"));
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
                             "%6"
                             "</center><br />"
                             "%7")
                     .arg(qApp->applicationName())
                     .arg(qApp->applicationVersion())
                     .arg(APP_DESC)
                     .arg(APP_COPY)
                     .arg("URL: <a href=\"https://launchpad.net/stackandconquer\">"
                          "https://launchpad.net/stackandconquer</a>")
                     .arg(trUtf8("License") +
                          ": "
                          "<a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                          "GNU General Public License Version 3</a>")
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
      // TODO: Update labels in GraphView
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
