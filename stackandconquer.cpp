/**
 * \file stackandconquer.cpp
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
  this->checkCmdArgs(sListArgs);
}

StackAndConquer::~StackAndConquer() = default;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::checkCmdArgs(const QStringList &sListArgs) {
  // Choose CPU script(s) or load game from command line
  QStringList sListTemp;
  if (!sListArgs.isEmpty()) {
    for (int i = 0; i < sListArgs.size(); i++) {
      // Load save game
      if (sListArgs.at(i).endsWith(QStringLiteral(".stacksav"),
                                   Qt::CaseInsensitive)) {
        if (QFile::exists(sListArgs.at(i))) {
          sListTemp.clear();
          sListTemp << sListArgs[i];
          break;
        }
        qWarning() << "Specified JS file not found:" << sListArgs[i];
        QMessageBox::warning(this, tr("Warning"),
                             tr("Specified file not found:") + "\n" +
                             sListArgs[i]);
        sListTemp.clear();
        break;
      }
      if (sListArgs.at(i).endsWith(QStringLiteral(".js"),
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
  m_plblPlayer1 = new QLabel(m_pSettings->getPlayerName(1));
  m_plblPlayer1->setStyleSheet(QStringLiteral("color: ") +
                               m_pSettings->getTextColor().name());
  m_plblP1StonesLeft = new QLabel(QStringLiteral("99"));
  m_plblP1StonesLeft->setStyleSheet(QStringLiteral("color: ") +
                                    m_pSettings->getTextColor().name());
  m_plblP1Won = new QLabel(QStringLiteral("0"));
  m_plblP1Won->setStyleSheet(QStringLiteral("color: ") +
                             m_pSettings->getTextColor().name());
  m_plblPlayer2 = new QLabel(m_pSettings->getPlayerName(2));
  m_plblPlayer2->setStyleSheet(QStringLiteral("color: ") +
                               m_pSettings->getTextColor().name());
  m_plblPlayer2->setAlignment(Qt::AlignRight);
  m_plblP2StonesLeft = new QLabel(QStringLiteral("99"));
  m_plblP2StonesLeft->setStyleSheet(QStringLiteral("color: ") +
                                    m_pSettings->getTextColor().name());
  m_plblP2StonesLeft->setAlignment(Qt::AlignRight);
  m_plblP2Won = new QLabel(QStringLiteral("0"));
  m_plblP2Won->setStyleSheet(QStringLiteral("color: ") +
                             m_pSettings->getTextColor().name());
  m_plblP2Won->setAlignment(Qt::AlignRight);

  QPixmap iconStone(16, 16);
  iconStone.fill(m_pSettings->getBgColor());
  QPainter *paint = new QPainter(&iconStone);
  paint->setPen(QPen(Qt::black));
  paint->setBrush(QBrush(QColor(m_pSettings->getPlayerColor(1))));
  paint->drawEllipse(0, 0, 15, 15);
  m_plblIconStones1 = new QLabel();
  m_plblIconStones1->setPixmap(iconStone);
  m_plblIconStones1->setAlignment(Qt::AlignCenter);
  paint->setPen(QPen(Qt::black));
  paint->setBrush(QBrush(QColor(m_pSettings->getPlayerColor(2))));
  paint->drawEllipse(0, 0, 15, 15);
  m_plblIconStones2 = new QLabel();
  m_plblIconStones2->setPixmap(iconStone);
  m_plblIconStones2->setAlignment(Qt::AlignCenter);
  delete paint;

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

  m_pFrame->setFixedWidth(this->width());
  m_pFrame->setLayout(m_pLayout);
  m_pLayout->setColumnStretch(1, 1);
  m_pLayout->setColumnStretch(2, 1);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::resizeEvent(QResizeEvent *pEvent) {
  m_pFrame->setFixedWidth(pEvent->size().width());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::startNewGame(const QStringList &sListArgs) {
  delete m_pGame;
  if (sListArgs.isEmpty()) {
    m_pGame = new Game(m_pSettings, m_pSettings->getBoardFile());
  } else {
    m_pGame = new Game(m_pSettings, sListArgs);
  }

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
  m_pGraphView->updateSceneRect(m_pGame->getScene()->sceneRect());
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
  }
  if (bP2Won) {
    m_pUi->statusBar->showMessage(
          tr("%1 won the game!").arg(m_plblPlayer2->text()));
    return;
  }

  if (bPlayer1) {
    m_plblPlayer1->setStyleSheet(QStringLiteral("color: ") +
                                 m_pSettings->getTextHighlightColor().name());
    m_plblPlayer2->setStyleSheet(QStringLiteral("color: ") +
                                 m_pSettings->getTextColor().name());
    m_pUi->statusBar->showMessage(
          tr("%1's turn").arg(m_plblPlayer1->text()));
  } else {
    m_plblPlayer1->setStyleSheet(QStringLiteral("color: ") +
                                 m_pSettings->getTextColor().name());
    m_plblPlayer2->setStyleSheet(QStringLiteral("color: ") +
                                 m_pSettings->getTextHighlightColor().name());
    m_pUi->statusBar->showMessage(
          tr("%1's turn").arg(m_plblPlayer2->text()));
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void StackAndConquer::loadLanguage(const QString &sLang) {
  if (m_sCurrLang != sLang) {
    m_sCurrLang = sLang;
    if (!StackAndConquer::switchTranslator(&m_translatorQt, "qt_" + sLang,
                                           QLibraryInfo::location(
                                             QLibraryInfo::TranslationsPath))) {
      StackAndConquer::switchTranslator(&m_translatorQt, "qt_" + sLang,
                                        m_sSharePath + "/lang");
    }
    if (!StackAndConquer::switchTranslator(
          &m_translator,
           ":/" + qApp->applicationName().toLower() + "_" + sLang + ".qm")) {
      StackAndConquer::switchTranslator(
            &m_translator, qApp->applicationName().toLower() + "_" + sLang,
            m_sSharePath + "/lang");
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto StackAndConquer::switchTranslator(QTranslator *translator,
                                       const QString &sFile,
                                       const QString &sPath) -> bool {
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
  auto *dialog = new QDialog(this, this->windowFlags()
                             & ~Qt::WindowContextHelpButtonHint);
  auto *layout = new QGridLayout(dialog);
  dialog->setWindowTitle(tr("Rules"));
  dialog->setMinimumSize(700, 450);

  auto *textEdit = new QTextEdit;
  textEdit->setReadOnly(true);
  auto *credits = new QLabel;
  credits->setOpenExternalLinks(true);

  layout->setContentsMargins(2, 2, 2, 2);
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

void StackAndConquer::reportBug() {
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
             ": <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">"
             "GNU General Public License Version 3</a>",
             tr("This application uses icons from "
                "<a href=\"http://tango.freedesktop.org\">"
                "Tango project</a>.") + "<br/>" +
             tr("The game is based on "
                "<a href=\"https://spielstein.com/games/mixtour\">"
                "Mixtour</a> by Dieter Stein."),
             "<i>" + tr("Translations") +
             "</i><br />"
             "&nbsp;&nbsp;- Dutch: Vistaus<br />"
             "&nbsp;&nbsp;- German: ElThoro<br />"
             "&nbsp;&nbsp;- Italian: davi92"));
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
