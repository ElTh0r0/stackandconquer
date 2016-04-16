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
 * Main application generation (gui, object creation etc.).
 */

#include <QApplication>
#include <QGridLayout>
#include <QMessageBox>

#include <QInputDialog>

#include "./CStackAndConquer.h"
#include "ui_CStackAndConquer.h"

CStackAndConquer::CStackAndConquer(const QDir &sharePath, QWidget *pParent)
    : QMainWindow(pParent),
      m_pUi(new Ui::CStackAndConquer),
      m_pBoard(NULL),
      m_pPlayer1(NULL),
      m_pPlayer2(NULL),
      m_nMaxTowerHeight(5),
      m_nMaxStones(20),
      m_nGridSize(70),
      m_sSharePath(sharePath.absolutePath()) {
    qDebug() << Q_FUNC_INFO;

    m_pUi->setupUi(this);
    this->setWindowTitle(qApp->applicationName());
    m_pSettings = new CSettings(m_sSharePath, this);
    this->setupMenu();

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

    m_pFrame1 = new QFrame(m_pGraphView);
    m_pLayout1 = new QFormLayout;
    m_plblPlayer1 = new QLabel(m_pSettings->getNameP1());
    m_plblPlayer1StonesLeft = new QLabel(trUtf8("Stones left:") + " " + QString::number(m_nMaxStones));
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
    m_plblPlayer2StonesLeft = new QLabel(QString::number(m_nMaxStones));
    m_plblPlayer2Won = new QLabel("0");
    m_pLayout2->setVerticalSpacing(0);
    m_pLayout2->addRow(m_plblPlayer2);
    m_pLayout2->addRow(trUtf8("Stones left:"), m_plblPlayer2StonesLeft);
    m_pLayout2->addRow(trUtf8("Won:"), m_plblPlayer2Won);
    m_pFrame2->setLayout(m_pLayout2);
    m_pFrame2->move(this->width() - m_pLayout2->sizeHint().width(), 0);

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

void CStackAndConquer::startNewGame() {
    qDebug() << Q_FUNC_INFO;

        if (NULL != m_pBoard) {
            delete m_pBoard;
        }
        m_pBoard = new CBoard(m_pGraphView, m_nGridSize, m_nMaxStones,
                              m_pSettings);
        m_pGraphView->setScene(m_pBoard);
        connect(m_pBoard, SIGNAL(setStone(QPoint)),
                this, SLOT(setStone(QPoint)));
        connect(m_pBoard, SIGNAL(moveTower(QPoint, QPoint)),
                this, SLOT(moveTower(QPoint, QPoint)));

        if (NULL != m_pPlayer1) {
            delete m_pPlayer1;
        }
        if (NULL != m_pPlayer2) {
            delete m_pPlayer2;
        }

        QString sP2HumanCpu(m_pSettings->getP2HumanCpu());
        bool bP2HumanCpu(true);
        if ("Human" == sP2HumanCpu) {
            bP2HumanCpu = true;
        } else {
            // TODO: Implement CPU opponent
            qWarning() << "CPU Player not implemeted, yet!";
            // bP2HumanCpu = false;
        }

        // Select start player
        quint8 nStartPlayer(m_pSettings->getStartPlaner());
        bool bStart1(true);
        bool bStart2(false);
        if (0 == nStartPlayer) {  // Random
            nStartPlayer = qrand() % 2 + 1;
        }
        if (2 == nStartPlayer) {  // Player 2
            bStart1 = false;
            bStart2 = true;
        } else {  // Player 1
            bStart1 = true;
            bStart2 = false;
        }

        m_pPlayer1 = new CPlayer(bStart1, true,
                                 m_pSettings->getNameP1(), m_nMaxStones);
        m_pPlayer2 = new CPlayer(bStart2, bP2HumanCpu,
                                 m_pSettings->getNameP2(), m_nMaxStones);
        m_plblPlayer1->setText(m_pSettings->getNameP1());
        m_plblPlayer2->setText(m_pSettings->getNameP2());

        // m_pUi->action_SaveGame->setEnabled(true);
        m_pGraphView->setInteractive(true);
        m_sPreviousMove.clear();
        this->updatePlayers(true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::setStone(QPoint field) {
    QString sMove(static_cast<char>(field.x() + 65)
                  + QString::number(field.y() + 1));

    if (0 == m_pBoard->getField(field).size()) {
        if (m_pPlayer1->getIsActive() && m_pPlayer1->getStonesLeft() > 0) {
            m_pPlayer1->setStonesLeft(m_pPlayer1->getStonesLeft() - 1);
            m_pBoard->addStone(field, 1);
            qDebug() << "P1 >>" << sMove;
        } else if (m_pPlayer2->getIsActive() && m_pPlayer2->getStonesLeft() > 0) {
            m_pPlayer2->setStonesLeft(m_pPlayer2->getStonesLeft() - 1);
            m_pBoard->addStone(field, 2);
            qDebug() << "P2 >>" << sMove;
        } else {
            QMessageBox::information(NULL, trUtf8("Information"),
                                     trUtf8("No stones left! Please move a tower."));
            return;
        }
        m_sPreviousMove.clear();

        this->checkTowerWin(field);
        this->updatePlayers();
    } else {
        QMessageBox::information(NULL, trUtf8("Information"),
                                 trUtf8("It is only allowed to place a "
                                        "stone on a free field."));
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::moveTower(QPoint tower, QPoint moveTo) {
    QList<quint8> listStones = m_pBoard->getField(tower);
    if (0 == listStones.size()) {
        qWarning() << "Move tower size == 0!";
        QMessageBox::warning(NULL, trUtf8("Warning"),
                             trUtf8("Something went wrong!"));
        return;
    }

    int nStonesToMove = 1;
    if (listStones.size() > 1) {
        bool ok;
        nStonesToMove = QInputDialog::getInt(this, trUtf8("Move tower"),
                                             trUtf8("How many stones shall be moved:"),
                                             1, 1, listStones.size(), 1, &ok);
        if (!ok) {
            return;
        }
    }

    // Debug print: E.g. "C4:3-D3" = move 3 stones from C4 to D3 (ASCII 65 = A)
    QString sMove(static_cast<char>(tower.x() + 65) + QString::number(tower.y() + 1)
                  + ":" + QString::number(nStonesToMove) + "-"
                  + static_cast<char>(moveTo.x() + 65) + QString::number(moveTo.y() + 1));
    if (this->checkPreviousMoveReverted(sMove)) {
        QMessageBox::information(NULL, trUtf8("Information"),
                                 trUtf8("It is not allowed to revert the "
                                        "previous oppenents move directly!"));
        return;
    }

    if (m_pPlayer1->getIsActive()) {
        qDebug() << "P1 >>" << sMove;
    } else {
        qDebug() << "P2 >>" << sMove;
    }

    for (int i = 0; i < nStonesToMove; i++) {
        m_pBoard->removeStone(tower);  // Remove is in the wrong order, nevermind!
        m_pBoard->addStone(moveTo,
                           listStones[listStones.size() - nStonesToMove + i]);
    }

    this->checkTowerWin(moveTo);
    this->updatePlayers();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::checkTowerWin(QPoint field) {
    if (m_pBoard->getField(field).size() >= m_nMaxTowerHeight) {
        if (1 == m_pBoard->getField(field).last()) {
            m_pPlayer1->increaseWonTowers();
            qDebug() << "Player 1 won tower" << field;
        } else if (2 == m_pBoard->getField(field).last()) {
            m_pPlayer2->increaseWonTowers();
            qDebug() << "Player 2 won tower" << field;
        } else {
            qDebug() << Q_FUNC_INFO;
            qWarning() << "Last stone neither 1 nor 2!";
            qWarning() << "Field:" << field
                       << " -  Tower" << m_pBoard->getField(field);
            QMessageBox::warning(NULL, trUtf8("Warning"),
                                 trUtf8("Something went wrong!"));
            return;
        }
        this->returnStones(field);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::returnStones(QPoint field) {
    QList<quint8> tower = m_pBoard->getField(field);
    quint8 stones = tower.count(1);
    m_pPlayer1->setStonesLeft(m_pPlayer1->getStonesLeft() + stones);
    stones = tower.count(2);
    m_pPlayer2->setStonesLeft(m_pPlayer2->getStonesLeft() + stones);

    // Clear field
    m_pBoard->removeStone(field, true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::updatePlayers(bool bInitial) {
    m_plblPlayer1StonesLeft->setText(trUtf8("Stones left:") + " "  +
                                     QString::number(m_pPlayer1->getStonesLeft()));
    m_plblPlayer2StonesLeft->setText(QString::number(m_pPlayer2->getStonesLeft()));
    m_plblPlayer1Won->setText(trUtf8("Won:") + " "  +
                                     QString::number(m_pPlayer1->getWonTowers()));
    m_plblPlayer2Won->setText(QString::number(m_pPlayer2->getWonTowers()));

    if (m_pSettings->getWinTowers() == m_pPlayer1->getWonTowers()) {
        qDebug() << "PLAYER 1 WON!";
        m_pGraphView->setInteractive(false);
        QMessageBox::information(this, qApp->applicationName(),
                                 trUtf8("%1 won the game!")
                                 .arg(m_pPlayer1->getName()));
    } else if (m_pSettings->getWinTowers() == m_pPlayer2->getWonTowers()) {
        qDebug() << "PLAYER 2 WON!";
        m_pGraphView->setInteractive(false);
        QMessageBox::information(this, qApp->applicationName(),
                                 trUtf8("%1 won the game!")
                                 .arg(m_pPlayer2->getName()));
    } else {
        if (!bInitial) {
            m_pPlayer1->setActive(!m_pPlayer1->getIsActive());
            m_pPlayer2->setActive(!m_pPlayer2->getIsActive());
        }

        if (m_pPlayer1->getIsActive()) {
            m_plblPlayer1->setStyleSheet("color: #FF0000");
            m_plblPlayer2->setStyleSheet("color: #000000");
        } else {
            m_plblPlayer1->setStyleSheet("color: #000000");
            m_plblPlayer2->setStyleSheet("color: #FF0000");
        }
        this->checkPossibleMoves();
    }
    m_pBoard->printDebugFields();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CStackAndConquer::checkPossibleMoves() {
    if (m_pPlayer1->getIsActive()) {
        if (m_pBoard->findPossibleMoves(m_pPlayer1->getStonesLeft() > 0)) {
            m_pPlayer1->setCanMove(true);
            return;
        } else {
            m_pPlayer1->setCanMove(false);
        }
    } else {
        if (m_pBoard->findPossibleMoves(m_pPlayer2->getStonesLeft() > 0)) {
            m_pPlayer2->setCanMove(true);
            return;
        } else {
            m_pPlayer2->setCanMove(false);
        }
    }

    if (!m_pPlayer1->getCanMove() && !m_pPlayer2->getCanMove()) {
        m_pGraphView->setInteractive(false);
        qDebug() << "NO MOVES POSSIBLE ANYMORE!";
        QMessageBox::information(this, qApp->applicationName(),
                                 trUtf8("No moves possible anymore.\n"
                                        "Game ends in a tie!"));
    } else if (!m_pPlayer1->getCanMove()) {
        qDebug() << "PLAYER 1 HAS TO PASS!";
        QMessageBox::information(this, qApp->applicationName(),
                                 trUtf8("No move possible!\n%1 has to pass.")
                                 .arg(m_pPlayer1->getName()));
        this->updatePlayers();
    } else if (!m_pPlayer2->getCanMove()) {
        qDebug() << "PLAYER 2 HAS TO PASS!";
        QMessageBox::information(this, qApp->applicationName(),
                                 trUtf8("No move possible!\n%1 has to pass.")
                                 .arg(m_pPlayer2->getName()));
        this->updatePlayers();
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CStackAndConquer::checkPreviousMoveReverted(const QString sMove) {
    if (!m_sPreviousMove.isEmpty()) {
        QStringList sListPrev;
        QStringList sListCur;
        sListPrev = m_sPreviousMove.split(":");
        sListCur = sMove.split(":");

        if (2 == sListPrev.size() && 2 == sListCur.size()) {
            QString tmp(sListPrev[1]);
            sListPrev.removeLast();
            sListPrev << tmp.split("-");
            tmp = sListCur[1];
            sListCur.removeLast();
            sListCur << tmp.split("-");

            if (3 == sListPrev.size() && 3 == sListCur.size()) {
                if (sListPrev[0] == sListCur[2]
                        && sListPrev[1] == sListCur[1]
                        && sListPrev[2] == sListCur[0]) {
                    return true;
                }
            } else {
                qDebug() << Q_FUNC_INFO;
                qWarning() << "Splitting 2 failed:" << m_sPreviousMove << sMove;
                QMessageBox::warning(NULL, trUtf8("Warning"),
                                     trUtf8("Something went wrong!"));
                return false;
            }
        } else {
            qDebug() << Q_FUNC_INFO;
            qWarning() << "Splitting 1 failed:" << m_sPreviousMove << sMove;
            QMessageBox::warning(NULL, trUtf8("Warning"),
                                 trUtf8("Something went wrong!"));
            return false;
        }
    }

    m_sPreviousMove = sMove;
    return false;
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
                       .arg(trUtf8("License") + ": "
                            "<a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                            "GNU General Public License Version 3</a>")
                       .arg("<i>" + trUtf8("Translations") + "</i><br />"
                            "&nbsp;&nbsp;- German: ElThoro"));
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
