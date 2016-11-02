/**
 * \file CGame.cpp
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
 * Main game engine (object creation etc.).
 */

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>

#include "./CGame.h"

CGame::CGame(CSettings *pSettings)
  : m_pSettings(pSettings),
    m_pBoard(NULL),
    m_pPlayer1(NULL),
    m_pPlayer2(NULL),
    m_nMaxTowerHeight(5),
    m_nMaxStones(20),
    m_nGridSize(70),
    m_nNumOfFields(5) {
  qDebug() << Q_FUNC_INFO;

  if (NULL != m_pBoard) {
    delete m_pBoard;
  }
  m_pBoard = new CBoard(m_nNumOfFields, m_nGridSize, m_nMaxStones, m_pSettings);

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
  quint8 nStartPlayer(m_pSettings->getStartPlayer());
  bool bStartPlayer(true);
  if (0 == nStartPlayer) {  // Random
    nStartPlayer = qrand() % 2 + 1;
  }
  if (2 == nStartPlayer) {  // Player 2
    bStartPlayer = false;
  } else {  // Player 1
    bStartPlayer = true;
  }

  m_pPlayer1 = new CPlayer(bStartPlayer, true,
                           m_pSettings->getNameP1(), m_nMaxStones);
  m_pPlayer2 = new CPlayer(!bStartPlayer, bP2HumanCpu,
                           m_pSettings->getNameP2(), m_nMaxStones);

  // m_pUi->action_SaveGame->setEnabled(true);
  m_sPreviousMove.clear();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QGraphicsScene* CGame::getScene() {
  return m_pBoard;
}

QRectF CGame::getSceneRect() {
  return QRectF(0, 0,
                m_nNumOfFields * m_nGridSize-1, m_nNumOfFields * m_nGridSize-1);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CGame::setStone(QPoint field) {
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

void CGame::moveTower(QPoint tower, QPoint moveTo) {
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
    nStonesToMove = QInputDialog::getInt(NULL, trUtf8("Move tower"),
                                         trUtf8("How many stones shall be moved:"),
                                         1, 1, listStones.size(), 1, &ok);
    if (!ok) {
      return;
    }
  }

  // Debug print: E.g. "C4:3-D3" = move 3 stones from C4 to D3 (ASCII 65 = A)
  QString sMove(static_cast<char>(tower.x() + 65) +
                QString::number(tower.y() + 1) + ":" +
                QString::number(nStonesToMove) + "-" +
                static_cast<char>(moveTo.x() + 65) +
                QString::number(moveTo.y() + 1));
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

void CGame::checkTowerWin(QPoint field) {
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

void CGame::returnStones(QPoint field) {
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

void CGame::updatePlayers(bool bInitial) {
  emit updateNameP1(m_pSettings->getNameP1());
  emit updateNameP2(m_pSettings->getNameP2());

  emit updateStonesP1(trUtf8("Stones left:") + " "  +
                      QString::number(m_pPlayer1->getStonesLeft()));
  emit updateStonesP2(QString::number(m_pPlayer2->getStonesLeft()));

  emit updateWonP1(trUtf8("Won:") + " "  +
                   QString::number(m_pPlayer1->getWonTowers()));
  emit updateWonP2(QString::number(m_pPlayer2->getWonTowers()));


  if (m_pSettings->getWinTowers() == m_pPlayer1->getWonTowers()) {
    qDebug() << "PLAYER 1 WON!";
    emit setInteractive(false);
    QMessageBox::information(NULL, trUtf8("Information"),
                             trUtf8("%1 won the game!")
                             .arg(m_pPlayer1->getName()));
  } else if (m_pSettings->getWinTowers() == m_pPlayer2->getWonTowers()) {
    qDebug() << "PLAYER 2 WON!";
    emit setInteractive(false);
    QMessageBox::information(NULL, trUtf8("Information"),
                             trUtf8("%1 won the game!")
                             .arg(m_pPlayer2->getName()));
  } else {
    if (!bInitial) {
      m_pPlayer1->setActive(!m_pPlayer1->getIsActive());
      m_pPlayer2->setActive(!m_pPlayer2->getIsActive());
    }

    emit highlightActivePlayer(m_pPlayer1->getIsActive());
    this->checkPossibleMoves();
  }

  m_pBoard->printDebugFields();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CGame::checkPossibleMoves() {
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
    emit setInteractive(false);
    qDebug() << "NO MOVES POSSIBLE ANYMORE!";
    QMessageBox::information(NULL, trUtf8("Information"),
                             trUtf8("No moves possible anymore.\n"
                                    "Game ends in a tie!"));
  } else if (!m_pPlayer1->getCanMove()) {
    qDebug() << "PLAYER 1 HAS TO PASS!";
    QMessageBox::information(NULL, trUtf8("Information"),
                             trUtf8("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer1->getName()));
    this->updatePlayers();
  } else if (!m_pPlayer2->getCanMove()) {
    qDebug() << "PLAYER 2 HAS TO PASS!";
    QMessageBox::information(NULL, trUtf8("Information"),
                             trUtf8("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer2->getName()));
    this->updatePlayers();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool CGame::checkPreviousMoveReverted(const QString sMove) {
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
        if (sListPrev[0] == sListCur[2] &&
            sListPrev[1] == sListCur[1] &&
            sListPrev[2] == sListCur[0]) {
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
