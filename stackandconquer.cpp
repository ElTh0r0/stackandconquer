/**
 * \file stackandconquer.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2021 Thorsten Roth
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
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsView>
#include <QGridLayout>
#include <QLabel>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QResizeEvent>
#include <QTabWidget>
#include <QTextEdit>
#include <QSlider>

#include "./game.h"
#include "./settings.h"

#include "ui_stackandconquer.h"

StackAndConquer::StackAndConquer(const QDir &sharePath,
                                 const QDir &userDataPath,
                                 const QStringList &sListArgs,
                                 QWidget *pParent)
  : QMainWindow(pParent),
    m_pUi(new Ui::StackAndConquer),
    m_userDataDir(userDataPath),
    m_sSharePath(sharePath.absolutePath()),
    m_nMaxPlayers(2),
    m_sCurrLang(QString()),
    m_pGame(nullptr),
    // Size is based on default grid size of 70!
    m_DefaultSize(600, 480) {
  m_pUi->setupUi(this);

  m_pSettings = new Settings(m_sSharePath, m_userDataDir.absolutePath(),
                             m_nMaxPlayers, this);
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
  // Open savegame from command line
  QString sSavegame(QLatin1String(""));
  if (!sListArgs.isEmpty()) {
    // Load save game
    if (sListArgs.at(0).endsWith(QStringLiteral(".stacksav"),
                                 Qt::CaseInsensitive)) {
      if (QFile::exists(sListArgs.at(0))) {
        sSavegame = sListArgs.at(0);
      } else {
        qWarning() << "Specified savegame not found:" << sListArgs.at(0);
        QMessageBox::warning(this, tr("Warning"),
                             tr("Specified file not found:") + "\n" +
                             sListArgs.at(0));
      }
    }
  }
  this->startNewGame(sSavegame);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::setupMenu() {
  // Game menu
  m_pUi->action_NewGame->setShortcut(QKeySequence::New);
  connect(m_pUi->action_NewGame, &QAction::triggered,
          this, [this]() { this->startNewGame(QString()); });
  m_pUi->action_LoadGame->setShortcut(QKeySequence::Open);
  connect(m_pUi->action_LoadGame, &QAction::triggered,
          this, &StackAndConquer::loadGame);
  m_pUi->action_SaveGame->setShortcut(QKeySequence::Save);
  connect(m_pUi->action_SaveGame, &QAction::triggered,
          this, &StackAndConquer::saveGame);
  connect(m_pUi->action_Preferences, &QAction::triggered,
          m_pSettings, &Settings::show);
  m_pUi->action_Quit->setShortcut(QKeySequence::Quit);
  connect(m_pUi->action_Quit, &QAction::triggered,
          this, &StackAndConquer::close);

  // Help menu
  connect(m_pUi->action_Rules, &QAction::triggered,
          this, &StackAndConquer::showRules);
  connect(m_pUi->action_ReportBug, &QAction::triggered,
          this, &StackAndConquer::reportBug);
  connect(m_pUi->action_Info, &QAction::triggered,
          this, &StackAndConquer::showInfoBox);

  m_pZoomSlider = new QSlider(Qt::Orientation::Horizontal, this);
  m_pZoomSlider->setMinimum(m_pSettings->getDefaultGrid());
  m_pZoomSlider->setMaximum(m_pSettings->getMaxGrid());
  m_pZoomSlider->setSingleStep(5);
  m_pZoomSlider->setTickInterval(10);
  m_pZoomSlider->setTracking(false);
  m_pZoomSlider->setMaximumWidth(200);
  m_pZoomSlider->setValue(m_pSettings->getGridSize());
  m_pUi->statusBar->addPermanentWidget(m_pZoomSlider);
  connect(m_pZoomSlider, &QSlider::valueChanged,
          this, &StackAndConquer::zoomChanged);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::setupGraphView() {
  m_pGraphView = new QGraphicsView(this);
  // Set mouse tracking to true, otherwise mouse move event
  // for the *scene* is only triggered on a mouse click!
  // QGraphicsView forwards the event to the scene.
  m_pGraphView->setMouseTracking(true);

  // Transform coordinate system to "isometric" view
  QTransform transfISO;
  transfISO = transfISO.scale(1.0, 0.5).rotate(45);
  m_pGraphView->setTransform(transfISO);
  this->setCentralWidget(m_pGraphView);

  m_pFrame = new QFrame(m_pGraphView);
  m_pLayout = new QGridLayout;
  m_pLayout->setVerticalSpacing(0);
  QPixmap iconWin(QStringLiteral(":/img/win.png"));
  if (m_pSettings->getBgColor().lightnessF() < 0.5) {
    iconWin = QStringLiteral(":/img/win2.png");
  }

  for (int i = 0; i < m_nMaxPlayers; i++) {
    m_pLblsPlayerName << new QLabel(QStringLiteral("Player"));
    m_pLblsPlayerName.last()->setStyleSheet(QStringLiteral("color: ") +
                                            m_pSettings->getTextColor().name());
    QSizePolicy sp_retain = m_pLblsPlayerName.last()->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    m_pLblsPlayerName.last()->setSizePolicy(sp_retain);

    m_pLblsStoneIcon << new QLabel();
    this->drawPlayerIcon(i);
    m_pLblsStoneIcon.last()->setSizePolicy(sp_retain);
    m_pLblsStonesLeft << new QLabel(QStringLiteral("99"));
    m_pLblsStonesLeft.last()->setStyleSheet(QStringLiteral("color: ") +
                                            m_pSettings->getTextColor().name());
    m_pLblsStonesLeft.last()->setSizePolicy(sp_retain);

    m_pLblsWinIcon << new QLabel();
    m_pLblsWinIcon.last()->setPixmap(iconWin);
    m_pLblsWinIcon.last()->setAlignment(Qt::AlignCenter);
    m_pLblsWinIcon.last()->setSizePolicy(sp_retain);
    m_pLblsWon << new QLabel(QStringLiteral("0"));
    m_pLblsWon.last()->setStyleSheet(QStringLiteral("color: ") +
                                     m_pSettings->getTextColor().name());
    m_pLblsWon.last()->setSizePolicy(sp_retain);

    if (0 == i % 2) {
      // addWidget(*widget, row, column, rowspan, colspan)
      m_pLayout->addWidget(m_pLblsPlayerName.last(), i + ((i/2)*4), 0, 1, 2);
      m_pLayout->addWidget(m_pLblsStoneIcon.last(), i+1 + ((i/2)*4), 0, 1, 1);
      m_pLayout->addWidget(m_pLblsStonesLeft.last(), i+1 + ((i/2)*4), 1, 1, 1);
      m_pLayout->addWidget(m_pLblsWinIcon.last(), i+2 + ((i/2)*4), 0, 1, 1);
      m_pLayout->addWidget(m_pLblsWon.last(), i+2 + ((i/2)*4), 1, 1, 1);
    } else {
      m_pLayout->addWidget(m_pLblsPlayerName.last(), i-1 + ((i/2)*4), 2, 1, 2);
      m_pLayout->addWidget(m_pLblsStonesLeft.last(), i + ((i/2)*4), 2, 1, 1);
      m_pLayout->addWidget(m_pLblsStoneIcon.last(), i + ((i/2)*4), 3, 1, 1);
      m_pLayout->addWidget(m_pLblsWon.last(), i+1 + ((i/2)*4), 2, 1, 1);
      m_pLayout->addWidget(m_pLblsWinIcon.last(), i+1 + ((i/2)*4), 3, 1, 1);

      m_pLblsPlayerName.last()->setAlignment(Qt::AlignRight);
      m_pLblsStonesLeft.last()->setAlignment(Qt::AlignRight);
      m_pLblsWon.last()->setAlignment(Qt::AlignRight);
    }
  }

  this->zoomChanged(m_pSettings->getGridSize());
  m_pFrame->setLayout(m_pLayout);
  m_pLayout->setColumnStretch(1, 1);
  m_pLayout->setColumnStretch(2, 1);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::drawPlayerIcon(const quint8 nID) {
  QPixmap iconStone(16, 16);
  iconStone.fill(m_pSettings->getBgColor());
  QPainter *pPainter = new QPainter(&iconStone);
  pPainter->setPen(QPen(Qt::black));
  pPainter->setBrush(QBrush(QColor(m_pSettings->getPlayerColor(nID))));
  pPainter->drawEllipse(0, 0, 15, 15);
  m_pLblsStoneIcon[nID]->setPixmap(iconStone);
  m_pLblsStoneIcon[nID]->setAlignment(Qt::AlignCenter);
  delete pPainter;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::updateNames(const QStringList &sListName) {
  for (int i = 0; i < m_nMaxPlayers; i++) {
    if (i < sListName.size()) {
      m_pLblsPlayerName[i]->setText(sListName.at(i));
      m_pLblsPlayerName[i]->setVisible(true);
      m_pLblsStoneIcon[i]->setVisible(true);
      m_pLblsStonesLeft[i]->setVisible(true);
      m_pLblsWinIcon[i]->setVisible(true);
      m_pLblsWon[i]->setVisible(true);
    } else {
      m_pLblsPlayerName[i]->setVisible(false);
      m_pLblsStoneIcon[i]->setVisible(false);
      m_pLblsStonesLeft[i]->setVisible(false);
      m_pLblsWinIcon[i]->setVisible(false);
      m_pLblsWon[i]->setVisible(false);
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::zoomChanged(const int nNewGrid) {
  static bool bInitial = true;

  if (m_pSettings->getGridSize() != nNewGrid || bInitial) {
    m_pSettings->setGridSize(nNewGrid);
    this->resize(
          m_DefaultSize * m_pSettings->getGridSize() / m_pSettings->getDefaultGrid());
    m_pFrame->setFixedWidth(this->width());
    emit changeZoom();
    bInitial = false;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::resizeEvent(QResizeEvent *pEvent) {
  m_pFrame->setFixedWidth(pEvent->size().width());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void StackAndConquer::startNewGame(const QString &sSavegame) {
  delete m_pGame;
  m_pGame = new Game(m_pSettings, sSavegame);

  connect(m_pGame, &Game::updateNames, this, &StackAndConquer::updateNames);
  connect(m_pGame, &Game::drawIcon, this, &StackAndConquer::drawPlayerIcon);
  connect(m_pGame, &Game::updateStones,
          this, [this](const quint8 nID, const QString &sStones) {
    m_pLblsStonesLeft[nID]->setText(sStones);
  });
  connect(m_pGame, &Game::updateWon,
          this, [this](const quint8 nID, const QString &sWon) {
    m_pLblsWon[nID]->setText(sWon);
  });

  connect(m_pGame, &Game::setInteractive,
          this, &StackAndConquer::setViewInteractive);
  connect(m_pGame, &Game::highlightActivePlayer,
          this, &StackAndConquer::highlightActivePlayer);
  connect(this, &StackAndConquer::changeZoom,
          m_pGame, &Game::changeZoom);

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
      this->startNewGame(sFile);
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

void StackAndConquer::highlightActivePlayer(const quint8 nActivePlayer,
                                            const quint8 nPlayerWon) {
  if (nPlayerWon > 0) {
    m_pUi->statusBar->showMessage(
          tr("%1 won the game!").arg(
            m_pLblsPlayerName.at(nPlayerWon - 1)->text()));
    return;
  }

  for (int i = 0; i < m_pLblsPlayerName.size(); i++) {
    if ((nActivePlayer - 1) == i) {
      m_pLblsPlayerName[i]->setStyleSheet(
            QStringLiteral("color: ") +
            m_pSettings->getTextHighlightColor().name());
      m_pUi->statusBar->showMessage(
            tr("%1's turn").arg(m_pLblsPlayerName.at(i)->text()));
    } else {
      m_pLblsPlayerName[i]->setStyleSheet(
            QStringLiteral("color: ") +
            m_pSettings->getTextColor().name());
    }
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void StackAndConquer::loadLanguage(const QString &sLang) {
  if (m_sCurrLang != sLang) {
    m_sCurrLang = sLang;
    if (!StackAndConquer::switchTranslator(&m_translatorQt, "qt_" + sLang,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                                           QLibraryInfo::path(
#else
                                           QLibraryInfo::location(
#endif
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
  m_pUi->retranslateUi(this);
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
  auto *pDialog = new QDialog(this, this->windowFlags()
                             & ~Qt::WindowContextHelpButtonHint);
  auto *pLayout = new QGridLayout(pDialog);
  pDialog->setWindowTitle(tr("Rules"));
  pDialog->setMinimumSize(700, 450);

  auto *pTextEditRules = new QTextEdit;
  pTextEditRules->setReadOnly(true);
  auto *pTextEditRulesATrois = new QTextEdit;
  pTextEditRulesATrois->setReadOnly(true);
  auto *pTabs = new QTabWidget(pDialog);
  pTabs->addTab(pTextEditRules, tr("Standard rules"));
  // TODO(x): Implement rules for > 2 players
  // TODO(x): Remove after implementation of > 2 players
  if (m_nMaxPlayers > 2) {
    pTabs->addTab(pTextEditRulesATrois, tr("Addition for > 2 players"));
  }
  auto *pCredits = new QLabel;
  pCredits->setOpenExternalLinks(true);

  pLayout->setContentsMargins(2, 2, 2, 2);
  pLayout->setSpacing(0);
  pLayout->addWidget(pTabs);
  pLayout->addWidget(pCredits);

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  // Since Qt 6 UTF-8 is used by default
  stream.setCodec("UTF-8");
#endif
  pTextEditRules->setHtml(stream.readAll());
  rules.close();

  rules.setFileName(":/rules_a_trois_" + sLang + ".html");
  if (!rules.exists()) {
    qWarning() << rules.fileName() << "does not exist. Loading EN fallback.";
    rules.setFileName(QStringLiteral(":/rules_a_trois_en.html"));
  }
  if (!rules.open(QFile::ReadOnly | QFile::Text)) {
    qWarning() << "Could not open rules a trois:" << rules.fileName();
    QMessageBox::warning(this, tr("Warning"), tr("Could not open rules!"));
    return;
  }
  stream.setDevice(&rules);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  // Since Qt 6 UTF-8 is used by default
  stream.setCodec("UTF-8");
#endif
  pTextEditRulesATrois->setHtml(stream.readAll());
  rules.close();

  pCredits->setText(
        "<p>" + tr("These rules are licensed under Creative Commons "
                   "<a href=\"https://creativecommons.org/licenses/by-nc/"
                   "4.0/\">Attribution-Noncommercial 4.0 International</a> "
                   "license.") +
        "<br />Designer: Dieter Stein, <a href=\"https://spielstein.com/games/"
        "mixtour/rules\">spielstein.com</a></p>");

  pDialog->show();
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
        QString::fromLatin1("<center>"
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
             "&nbsp;&nbsp;- Dutch: Vistaus, Elbert Pol<br />"
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
