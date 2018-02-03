/**
 * \file stackandconquer.cpp
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
 * Main application generation (gui)
 */

#include <QApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QTextEdit>

#include "./stackandconquer.h"
#include "ui_stackandconquer.h"

StackAndConquer::StackAndConquer(const QDir &sharePath,
                                 const QDir &userDataPath,
                                 QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::StackAndConquer),
    m_userDataDir(userDataPath),
    m_sSharePath(sharePath.absolutePath()),
    m_sCurrLang(""),
    m_pGame(NULL) {
  m_pUi->setupUi(this);
  this->setWindowTitle(qApp->applicationName());

  m_pSettings = new Settings(m_sSharePath, m_userDataDir.absolutePath(), this);
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

  this->checkCmdArgs();
}

StackAndConquer::~StackAndConquer() {
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::checkCmdArgs() {
  // Choose CPU script(s) or load game from command line
  QStringList sListArgs;
  if (qApp->arguments().size() > 1) {
    for (int i = 1; i < qApp->arguments().size(); i++) {
      // Load save game
      if (qApp->arguments()[i].endsWith(".stacksav", Qt::CaseInsensitive)) {
        if (QFile::exists(qApp->arguments()[i])) {
          sListArgs.clear();
          sListArgs << qApp->arguments()[i];
          break;
        } else {
          qWarning() << "Specified JS file not found:" << qApp->arguments()[i];
          QMessageBox::warning(this, trUtf8("Warning"),
                               trUtf8("Specified file not found:") + "\n" +
                               qApp->arguments()[i]);
          sListArgs.clear();
          break;
        }
      } else if (qApp->arguments()[i].endsWith(".js", Qt::CaseInsensitive)) {
        // Load CPU script(s)
        if (QFile::exists(qApp->arguments()[i])) {
          if (2 == sListArgs.size()) {
            break;
          }
          sListArgs << qApp->arguments()[i];
        } else {
          qWarning() << "Specified JS file not found:" << qApp->arguments()[i];
          QMessageBox::warning(this, trUtf8("Warning"),
                               trUtf8("Specified file not found:") + "\n" +
                               qApp->arguments()[i]);
          sListArgs.clear();
          break;
        }
      }
    }
  }

  this->startNewGame(sListArgs);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::setupMenu() {
  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, SIGNAL(triggered()),
          this, SLOT(startNewGame()));

  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  connect(m_pUi->action_LoadGame, SIGNAL(triggered()),
          this, SLOT(loadGame()));

  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  connect(m_pUi->action_SaveGame, SIGNAL(triggered()),
          this, SLOT(saveGame()));

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

void StackAndConquer::setupGraphView() {
  m_pGraphView = new QGraphicsView(this);
  // Set mouse tracking to true, otherwise mouse move event
  // for the *scene* is only triggered on a mouse click!
  // QGraphicsView forwards the event to the scene.
  m_pGraphView->setMouseTracking(true);

  // TODO(volunteer): Scalable window/board/stones
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
  m_plblPlayer2->setAlignment(Qt::AlignRight);
  m_plblP2StonesLeft = new QLabel("99");
  m_plblP2StonesLeft->setAlignment(Qt::AlignRight);
  m_plblP2Won = new QLabel("0");
  m_plblP2Won->setAlignment(Qt::AlignRight);

  QPixmap iconStone1(":/images/stone1.png");
  m_plblIconStones1 = new QLabel();
  m_plblIconStones1->setPixmap(iconStone1);
  m_plblIconStones1->setAlignment(Qt::AlignCenter);
  QPixmap iconStone2(":/images/stone2.png");
  m_plblIconStones2 = new QLabel();
  m_plblIconStones2->setPixmap(iconStone2);
  m_plblIconStones2->setAlignment(Qt::AlignCenter);
  QPixmap iconWin(":/images/win.png");
  m_plblIconWin1 = new QLabel();
  m_plblIconWin1->setPixmap(iconWin);
  m_plblIconWin1->setAlignment(Qt::AlignCenter);
  m_plblIconWin2 = new QLabel();
  m_plblIconWin2->setPixmap(iconWin);
  m_plblIconWin2->setAlignment(Qt::AlignCenter);

  // addWidget(*widget, row, column, rowspan, colspan)
  m_pLayout->addWidget(m_plblPlayer1, 0, 0, 1, 2);
  m_pLayout->addWidget(m_plblIconStones1, 1, 0, 1, 1);
  m_pLayout->addWidget(m_plblP1StonesLeft, 1, 1, 1, 1);
  m_pLayout->addWidget(m_plblIconWin1, 2, 0, 1, 1);
  m_pLayout->addWidget(m_plblP1Won, 2, 1, 1, 1);

  m_pLayout->addWidget(m_plblPlayer2, 0, 2, 1, 2);
  m_pLayout->addWidget(m_plblP2StonesLeft, 1, 2, 1, 1);
  m_pLayout->addWidget(m_plblIconStones2, 1, 3, 1, 1);
  m_pLayout->addWidget(m_plblP2Won, 2, 2, 1, 1);
  m_pLayout->addWidget(m_plblIconWin2, 2, 3, 1, 1);

  m_pFrame->setMinimumWidth(this->width());
  m_pFrame->setLayout(m_pLayout);
  m_pLayout->setColumnStretch(1, 1);
  m_pLayout->setColumnStretch(2, 1);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::startNewGame(const QStringList sListArgs) {
  if (NULL != m_pGame) {
    delete m_pGame;
  }
  m_pGame = new Game(m_pSettings, sListArgs);

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

  if (!m_pGame->initCpu()) {
    m_pGraphView->setInteractive(false);
    QMessageBox::warning(this, trUtf8("Warning"),
                         trUtf8("An error occured during CPU initialization."));
    return;
  }
  m_pGame->updatePlayers(true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::loadGame() {
  QString sFile = QFileDialog::getOpenFileName(
                    this, trUtf8("Load game"),
                    m_userDataDir.absolutePath(),
                    trUtf8("Save games") + "(*.stacksav)");
  if (!sFile.isEmpty()) {
    if (!sFile.endsWith(".stacksav", Qt::CaseInsensitive)) {
      QMessageBox::warning(this, trUtf8("Warning"),
                           trUtf8("Invalid save game file."));
    } else {
      this->startNewGame(QStringList() << sFile);
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::saveGame() {
  QString sFile = QFileDialog::getSaveFileName(
                    this, trUtf8("Save game"),
                    m_userDataDir.absolutePath(),
                    trUtf8("Save games") + "(*.stacksav)");
  if (!sFile.isEmpty()) {
    if (!sFile.endsWith(".stacksav", Qt::CaseInsensitive)) {
      sFile += ".stacksav";
    }
    if (!m_pGame->saveGame(sFile)) {
      QMessageBox::warning(this, trUtf8("Warning"),
                           trUtf8("Game could not be saved."));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::setViewInteractive(const bool bEnabled) {
  m_pGraphView->setInteractive(bEnabled);
  m_pUi->action_SaveGame->setEnabled(bEnabled);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::highlightActivePlayer(const bool bPlayer1,
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

void StackAndConquer::loadLanguage(const QString &sLang) {
  if (m_sCurrLang != sLang) {
    m_sCurrLang = sLang;
    if (!this->switchTranslator(&m_translatorQt, "qt_" + sLang,
                                QLibraryInfo::location(
                                  QLibraryInfo::TranslationsPath))) {
      this->switchTranslator(&m_translatorQt, "qt_" + sLang,
                             m_sSharePath + "/lang");
    }
    if (!this->switchTranslator(
          &m_translator,
           ":/" + qApp->applicationName().toLower() + "_" + sLang + ".qm")) {
      this->switchTranslator(
            &m_translator, qApp->applicationName().toLower() + "_" + sLang,
            m_sSharePath + "/lang");
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool StackAndConquer::switchTranslator(QTranslator *translator,
                                       const QString &sFile,
                                       const QString &sPath) {
  qApp->removeTranslator(translator);
  if (translator->load(sFile, sPath)) {
    qApp->installTranslator(translator);
  } else {
    if (!sFile.endsWith("_en") && !sFile.endsWith("_en.qm")) {
      // EN is build in translation -> no file
      qWarning() << "Could not find translation" << sFile << "in" << sPath;
    }
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::showRules() {
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

  QString sLang(m_pSettings->getLanguage());
  sLang = sLang.left(2);
  QFile rules(":/rules_" + sLang + ".html");
  if (!rules.exists()) {
    qWarning() << rules.fileName() << "does not exist. Loading EN fallback.";
    rules.setFileName(":/rules_en.html");
  }

  if (!rules.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Could not open rules:" << rules.fileName();
    QMessageBox::warning(this, trUtf8("Warning"),
                         trUtf8("Could not open rules!"));
    return;
  }
  QTextStream stream(&rules);
  stream.setCodec("UTF-8");
  textEdit->setHtml(stream.readAll());

  credits->setText(
        "<p>" + trUtf8("These rules are licensed under Creative Commons "
                       "<a href=\"https://creativecommons.org/licenses/by-nc/"
                       "4.0/\">Attribution-Noncommercial 4.0 International</a> "
                       "license.") +
        "<br />Designer: Dieter Stein, <a href=\"https://spielstein.com/games/"
        "mixtour/rules\">spielstein.com</a></p>");

  dialog->show();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::reportBug() const {
  QDesktopServices::openUrl(
        QUrl("https://github.com/ElTh0r0/stackandconquer/issues"));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void StackAndConquer::showInfoBox() {
  QMessageBox::about(
        this, trUtf8("About"),
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
             ": <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
             "GNU General Public License Version 3</a>")
        .arg(trUtf8("This application uses icons from "
                    "<a href=\"http://tango.freedesktop.org\">"
                    "Tango project</a>."))
        .arg("<i>" + trUtf8("Translations") +
             "</i><br />"
             "&nbsp;&nbsp;- Dutch: Vistaus<br />"
             "&nbsp;&nbsp;- German: ElThoro"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::changeEvent(QEvent *pEvent) {
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
void StackAndConquer::closeEvent(QCloseEvent *pEvent) {
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
