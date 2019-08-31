/**
 * \file stackandconquer.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2019 Thorsten Roth <elthoro@gmx.de>
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

#include "./stackandconquer.h"

#include <QApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QTextEdit>

#include "ui_stackandconquer.h"

StackAndConquer::StackAndConquer(const QDir &sharePath,
                                 const QDir &userDataPath,
                                 const QStringList &sListArgs,
                                 QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::StackAndConquer),
    m_userDataDir(userDataPath),
    m_sSharePath(sharePath.absolutePath()),
    m_sCurrLang(QString()),
    m_pGame(nullptr) {
  m_pUi->setupUi(this);
  this->setWindowTitle(qApp->applicationName());

  m_pSettings = new Settings(m_sSharePath, m_userDataDir.absolutePath(), this);
  connect(m_pSettings, &Settings::newGame,
          this, &StackAndConquer::startNewGame);
  connect(m_pSettings, &Settings::changeLang,
          this, &StackAndConquer::loadLanguage);
  connect(this, &StackAndConquer::updateUiLang,
          m_pSettings, &Settings::updateUiLang);
  this->loadLanguage(m_pSettings->getLanguage());

  this->setupMenu();
  this->setupGraphView();

  // Seed random number generator
  QTime time = QTime::currentTime();
  qsrand(static_cast<uint>(time.msec()));

  this->checkCmdArgs(sListArgs);
}

StackAndConquer::~StackAndConquer() {
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::checkCmdArgs(const QStringList &sListArgs) {
  // Choose CPU script(s) or load game from command line
  QStringList sListTemp;
  if (sListArgs.size() > 0) {
    for (int i = 0; i < sListArgs.size(); i++) {
      // Load save game
      if (sListArgs.at(i).endsWith(QStringLiteral(".stacksav"),
                                   Qt::CaseInsensitive)) {
        if (QFile::exists(sListArgs.at(i))) {
          sListTemp.clear();
          sListTemp << sListArgs[i];
          break;
        } else {
          qWarning() << "Specified JS file not found:" << sListArgs[i];
          QMessageBox::warning(this, tr("Warning"),
                               tr("Specified file not found:") + "\n" +
                               sListArgs[i]);
          sListTemp.clear();
          break;
        }
      } else if (sListArgs.at(i).endsWith(QStringLiteral(".js"),
                                          Qt::CaseInsensitive)) {
        // Load CPU script(s)
        if (QFile::exists(sListArgs.at(i))) {
          if (2 == sListTemp.size()) {
            break;
          }
          sListTemp << sListArgs[i];
        } else {
          qWarning() << "Specified JS file not found:" << sListArgs[i];
          QMessageBox::warning(this, tr("Warning"),
                               tr("Specified file not found:") + "\n" +
                               sListArgs[i]);
          sListTemp.clear();
          break;
        }
      }
    }
  }

  this->startNewGame(sListTemp);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::setupMenu() {
  // New game
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, &QAction::triggered,
          this, [this]() { this->startNewGame(QStringList()); });

  // Load game
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  connect(m_pUi->action_LoadGame, &QAction::triggered,
          this, &StackAndConquer::loadGame);

  // Save game
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  connect(m_pUi->action_SaveGame, &QAction::triggered,
          this, &StackAndConquer::saveGame);

  // Settings
  connect(m_pUi->action_Preferences, &QAction::triggered,
          m_pSettings, &Settings::show);

  // Exit game
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  connect(m_pUi->action_Quit, &QAction::triggered,
          this, &StackAndConquer::close);

  // Show rules
  connect(m_pUi->action_Rules, &QAction::triggered,
          this, &StackAndConquer::showRules);

  // Report bug
  connect(m_pUi->action_ReportBug, &QAction::triggered,
          this, &StackAndConquer::reportBug);

  // About
  connect(m_pUi->action_Info, &QAction::triggered,
          this, &StackAndConquer::showInfoBox);
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
  m_plblP1StonesLeft = new QLabel(QStringLiteral("99"));
  m_plblP1Won = new QLabel(QStringLiteral("0"));
  m_plblPlayer2 = new QLabel(m_pSettings->getNameP2());
  m_plblPlayer2->setAlignment(Qt::AlignRight);
  m_plblP2StonesLeft = new QLabel(QStringLiteral("99"));
  m_plblP2StonesLeft->setAlignment(Qt::AlignRight);
  m_plblP2Won = new QLabel(QStringLiteral("0"));
  m_plblP2Won->setAlignment(Qt::AlignRight);

  QPixmap iconStone1(QStringLiteral(":/images/stone1.png"));
  m_plblIconStones1 = new QLabel();
  m_plblIconStones1->setPixmap(iconStone1);
  m_plblIconStones1->setAlignment(Qt::AlignCenter);
  QPixmap iconStone2(QStringLiteral(":/images/stone2.png"));
  m_plblIconStones2 = new QLabel();
  m_plblIconStones2->setPixmap(iconStone2);
  m_plblIconStones2->setAlignment(Qt::AlignCenter);
  QPixmap iconWin(QStringLiteral(":/images/win.png"));
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

void StackAndConquer::startNewGame(const QStringList &sListArgs) {
  if (nullptr != m_pGame) {
    delete m_pGame;
  }
  m_pGame = new Game(m_pSettings, sListArgs);

  connect(m_pGame, &Game::updateNameP1, m_plblPlayer1, &QLabel::setText);
  connect(m_pGame, &Game::updateNameP2, m_plblPlayer2, &QLabel::setText);

  connect(m_pGame, &Game::updateStonesP1, m_plblP1StonesLeft, &QLabel::setText);
  connect(m_pGame, &Game::updateStonesP2, m_plblP2StonesLeft, &QLabel::setText);

  connect(m_pGame, &Game::updateWonP1, m_plblP1Won, &QLabel::setText);
  connect(m_pGame, &Game::updateWonP2, m_plblP2Won, &QLabel::setText);

  connect(m_pGame, &Game::setInteractive,
          this, &StackAndConquer::setViewInteractive);
  connect(m_pGame, &Game::highlightActivePlayer,
          this, &StackAndConquer::highlightActivePlayer);

  m_pGraphView->setScene(m_pGame->getScene());
  m_pGraphView->updateSceneRect(m_pGame->getSceneRect());
  m_pGraphView->setInteractive(true);

  if (!m_pGame->initCpu()) {
    m_pGraphView->setInteractive(false);
    QMessageBox::warning(this, tr("Warning"),
                         tr("An error occured during CPU initialization."));
    return;
  }
  m_pGame->updatePlayers(true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::loadGame() {
  QString sFile = QFileDialog::getOpenFileName(
                    this, tr("Load game"), m_userDataDir.absolutePath(),
                    tr("Save games") + "(*.stacksav)");
  if (!sFile.isEmpty()) {
    if (!sFile.endsWith(QStringLiteral(".stacksav"), Qt::CaseInsensitive)) {
      QMessageBox::warning(this, tr("Warning"), tr("Invalid save game file."));
    } else {
      this->startNewGame(QStringList() << sFile);
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::saveGame() {
  QString sFile = QFileDialog::getSaveFileName(
                    this, tr("Save game"), m_userDataDir.absolutePath(),
                    tr("Save games") + "(*.stacksav)");
  if (!sFile.isEmpty()) {
    if (!sFile.endsWith(QStringLiteral(".stacksav"), Qt::CaseInsensitive)) {
      sFile += QStringLiteral(".stacksav");
    }
    if (!m_pGame->saveGame(sFile)) {
      QMessageBox::warning(this, tr("Warning"), tr("Game could not be saved."));
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
          tr("%1 won the game!").arg(m_plblPlayer1->text()));
    return;
  } else if (bP2Won) {
    m_pUi->statusBar->showMessage(
          tr("%1 won the game!").arg(m_plblPlayer2->text()));
    return;
  }

  if (bPlayer1) {
    m_plblPlayer1->setStyleSheet(QStringLiteral("color: #FF0000"));
    m_plblPlayer2->setStyleSheet(QStringLiteral("color: #000000"));
    m_pUi->statusBar->showMessage(
          tr("%1's turn").arg(m_plblPlayer1->text()));
  } else {
    m_plblPlayer1->setStyleSheet(QStringLiteral("color: #000000"));
    m_plblPlayer2->setStyleSheet(QStringLiteral("color: #FF0000"));
    m_pUi->statusBar->showMessage(
          tr("%1's turn").arg(m_plblPlayer2->text()));
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
    if (!sFile.endsWith(QStringLiteral("_en")) &&
        !sFile.endsWith(QStringLiteral("_en.qm"))) {
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
  dialog->setWindowTitle(tr("Rules"));
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
    rules.setFileName(QStringLiteral(":/rules_en.html"));
  }

  if (!rules.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Could not open rules:" << rules.fileName();
    QMessageBox::warning(this, tr("Warning"), tr("Could not open rules!"));
    return;
  }
  QTextStream stream(&rules);
  stream.setCodec("UTF-8");
  textEdit->setHtml(stream.readAll());

  credits->setText(
        "<p>" + tr("These rules are licensed under Creative Commons "
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
        QUrl(
          QStringLiteral("https://github.com/ElTh0r0/stackandconquer/issues")));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void StackAndConquer::showInfoBox() {
  QMessageBox::about(
        this, tr("About"),
        QString("<center>"
                "<big><b>%1 %2</b></big><br/>"
                "%3<br/>"
                "<small>%4</small><br/><br/>"
                "%5<br/>"
                "%6<br/>"
                "<small>%7</small>"
                "</center><br/>"
                "%8")
        .arg(qApp->applicationName(),
             qApp->applicationVersion(),
             QStringLiteral(APP_DESC),
             QStringLiteral(APP_COPY),
             "URL: <a href=\"https://github.com/ElTh0r0/stackandconquer\">"
             "https://github.com/ElTh0r0/stackandconquer</a>",
             tr("License") +
             ": <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
             "GNU General Public License Version 3</a>",
             tr("This application uses icons from "
                "<a href=\"http://tango.freedesktop.org\">"
                "Tango project</a>."),
             "<i>" + tr("Translations") +
             "</i><br />"
             "&nbsp;&nbsp;- Dutch: Vistaus<br />"
             "&nbsp;&nbsp;- German: ElThoro"));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::changeEvent(QEvent *pEvent) {
  if (nullptr != pEvent) {
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
  int nRet = QMessageBox::question(this, tr("Quit") + " - " +
                                   qApp->applicationName(),
                                   tr("Do you really want to quit?"),
                                   QMessageBox::Yes | QMessageBox::No);

  if (QMessageBox::Yes == nRet) {
    pEvent->accept();
  } else {
    pEvent->ignore();
  }
  */
}
