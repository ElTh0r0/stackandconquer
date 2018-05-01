/**
 * \file game.cpp
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
 * Main game engine (object creation etc.).
 */

#include "./game.h"

#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMessageBox>
#include <QTimer>

Game::Game(Settings *pSettings, const QStringList &sListFiles)
  : m_pSettings(pSettings),
    m_pBoard(NULL),
    m_jsCpuP1(NULL),
    m_jsCpuP2(NULL),
    m_pPlayer1(NULL),
    m_pPlayer2(NULL),
    m_sJsFileP1(QStringLiteral("")),
    m_sJsFileP2(QStringLiteral("")),
    m_nMaxTowerHeight(5),
    m_nMaxStones(20),
    m_nGridSize(70),
    m_nNumOfFields(5),
    m_bScriptError(false) {
  qDebug() << "Starting new game" << sListFiles;

  m_pBoard = new Board(m_nNumOfFields, m_nGridSize, m_nMaxStones, m_pSettings);
  connect(m_pBoard, &Board::setStone, this, &Game::setStone);
  connect(m_pBoard, &Board::moveTower, this, &Game::moveTower);

  QString sP1HumanCpu(QStringLiteral(""));
  QString sName1(QStringLiteral("P1"));
  QString sP2HumanCpu(QStringLiteral(""));
  QString sName2(QStringLiteral("P2"));
  quint8 nStartPlayer(0);
  quint8 nStonesLeftP1(m_nMaxStones);
  quint8 nStonesLeftP2(m_nMaxStones);
  quint8 nWonP1(0);
  quint8 nWonP2(0);

  if (1 == sListFiles.size()) {
    if (sListFiles[0].endsWith(QStringLiteral(".stacksav"),
                               Qt::CaseInsensitive)) {  // Load
      QJsonObject jsonObj(this->loadGame(sListFiles[0]));
      if (jsonObj.isEmpty()) {
        qWarning() << "Save file is empty!";
        QMessageBox::critical(NULL, tr("Warning"),
                             tr("Error while opening save game."));
        exit(-1);
      }

      sP1HumanCpu = jsonObj[QStringLiteral("HumanCpu1")].toString().trimmed();
      sName1 = jsonObj[QStringLiteral("Name1")].toString().trimmed();
      nWonP1 = jsonObj[QStringLiteral("Won1")].toInt();
      sP2HumanCpu = jsonObj[QStringLiteral("HumanCpu2")].toString().trimmed();
      sName2 = jsonObj[QStringLiteral("Name2")].toString().trimmed();
      nWonP2 = jsonObj[QStringLiteral("Won2")].toInt();
      nStartPlayer = jsonObj[QStringLiteral("Current")].toInt();

      if (sP1HumanCpu.isEmpty() || sP2HumanCpu.isEmpty() ||
          sName1.isEmpty() || sName2.isEmpty()) {
        qWarning() << "Save game contains invalid data:"
                   << "sP1HumanCpu / sP2HumanCpu / sName1 / sName2 is empty.";
        QMessageBox::critical(NULL, tr("Warning"),
                              tr("Save game contains invalid data."));
        exit(-1);
      }

      // Convert json array to board
      QJsonArray jsBoard = jsonObj[QStringLiteral("Board")].toArray();
      QJsonArray jsTower;
      QJsonArray jsLine;
      QList<quint8> tower;
      QList<QList<quint8> > line;
      QList<QList<QList<quint8> > > board;
      board.reserve(m_nNumOfFields);

      if (m_nNumOfFields != jsBoard.size()) {
        qWarning() << "Save game contains invalid data"
                   << "(m_nNumOfFields != jsBoard.size):"
                   << m_nNumOfFields << "!=" << jsBoard.size();
        QMessageBox::critical(NULL, tr("Warning"),
                              tr("Save game contains invalid data."));
        exit(-1);
      }

      for (int i = 0; i < m_nNumOfFields; i++) {
        line.clear();
        jsLine = jsBoard.at(i).toArray();
        if (m_nNumOfFields != jsLine.size()) {
          qWarning() << "Save game contains invalid data"
                     << "(m_nNumOfFields != jsLine.size). Line" << i << "size:"
                     << m_nNumOfFields << "!=" << jsLine.size();
          QMessageBox::critical(NULL, tr("Warning"),
                                tr("Save game contains invalid data."));
          exit(-1);
        }
        for (int j = 0; j < m_nNumOfFields; j++) {
          tower.clear();
          jsTower = jsLine.at(j).toArray();
          foreach (QJsonValue n, jsTower) {
            tower << n.toDouble();
            (1 == tower.last()) ? nStonesLeftP1-- : nStonesLeftP2--;
          }
          line.append(tower);
        }
        board.append(line);
      }

      m_pBoard->setupSavegame(board);
    } else if (sListFiles[0].endsWith(QStringLiteral(".js"),
                                      Qt::CaseInsensitive)) {  // 1 CPU
      sP1HumanCpu = QStringLiteral("Human");
      sName1 = m_pSettings->getNameP1();
      sP2HumanCpu = sListFiles[0];
      sName2 = QStringLiteral("Computer");
      nStartPlayer = m_pSettings->getStartPlayer();
    }
  } else if (2 == sListFiles.size()) {  // 2 CPU players
    sP1HumanCpu = sListFiles[0];
    sName1 = QStringLiteral("Computer");
    sP2HumanCpu = sListFiles[1];
    sName2 = QStringLiteral("Computer");
    nStartPlayer = m_pSettings->getStartPlayer();
  } else {  // Start game with current settings
    sP1HumanCpu = m_pSettings->getP1HumanCpu();
    sName1 = m_pSettings->getNameP1();
    sP2HumanCpu = m_pSettings->getP2HumanCpu();
    sName2 = m_pSettings->getNameP2();
    nStartPlayer = m_pSettings->getStartPlayer();
  }

  bool bP1IsHuman(true);
  bool bP2IsHuman(true);

  if ("Human" == sP1HumanCpu) {
    bP1IsHuman = true;
  } else {
    bP1IsHuman = false;
    m_sJsFileP1 = sP1HumanCpu;
    this->createCPU1();
  }

  if ("Human" == sP2HumanCpu) {
    bP2IsHuman = true;
  } else {
    bP2IsHuman = false;
    m_sJsFileP2 = sP2HumanCpu;
    this->createCPU2();
  }

  // Select start player
  bool bStartPlayer(true);
  if (0 == nStartPlayer) {  // Random
    nStartPlayer = qrand() % 2 + 1;
  }
  if (2 == nStartPlayer) {  // Player 2
    bStartPlayer = false;
  } else {  // Player 1
    bStartPlayer = true;
  }

  m_pPlayer1 = new Player(bStartPlayer, bP1IsHuman, sName1, m_nMaxStones);
  m_pPlayer2 = new Player(!bStartPlayer, bP2IsHuman, sName2, m_nMaxStones);
  m_pPlayer1->setStonesLeft(nStonesLeftP1);
  m_pPlayer1->setWonTowers(nWonP1);
  m_pPlayer2->setStonesLeft(nStonesLeftP2);
  m_pPlayer2->setWonTowers(nWonP2);

  m_sPreviousMove.clear();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::createCPU1() {
  m_jsCpuP1 = new OpponentJS(1, m_nNumOfFields, m_nMaxTowerHeight);
  connect(this, &Game::makeMoveCpuP1, m_jsCpuP1, &OpponentJS::makeMoveCpu);
  connect(m_jsCpuP1, &OpponentJS::setStone, this, &Game::setStone);
  connect(m_jsCpuP1, &OpponentJS::moveTower, this, &Game::moveTower);
  connect(m_jsCpuP1, &OpponentJS::scriptError, this, &Game::caughtScriptError);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::createCPU2() {
  m_jsCpuP2 = new OpponentJS(2, m_nNumOfFields, m_nMaxTowerHeight);
  connect(this, &Game::makeMoveCpuP2, m_jsCpuP2, &OpponentJS::makeMoveCpu);
  connect(m_jsCpuP2, &OpponentJS::setStone, this, &Game::setStone);
  connect(m_jsCpuP2, &OpponentJS::moveTower, this, &Game::moveTower);
  connect(m_jsCpuP2, &OpponentJS::scriptError, this, &Game::caughtScriptError);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Game::initCpu() {
  if (!m_pPlayer1->getIsHuman()) {
    if (!m_jsCpuP1->loadAndEvalCpuScript(m_sJsFileP1)) {
      return false;
    }
  }
  if (!m_pPlayer2->getIsHuman()) {
    if (!m_jsCpuP2->loadAndEvalCpuScript(m_sJsFileP2)) {
      return false;
    }
  }
  return true;
}

void Game::caughtScriptError() {
  m_bScriptError = true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QGraphicsScene* Game::getScene() const {
  return m_pBoard;
}

QRectF Game::getSceneRect() const {
  return QRectF(0, 0,
                m_nNumOfFields * m_nGridSize-1, m_nNumOfFields * m_nGridSize-1);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::setStone(QPoint field) {
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
      if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
          (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
        QMessageBox::information(
              NULL, tr("Information"),
              tr("No stones left! Please move a tower."));
      } else {
        m_bScriptError = true;
        qWarning() << "CPU tried to set stone, but no stones left!";
        QMessageBox::warning(NULL, tr("Warning"),
                             tr("CPU script made an invalid move! "
                                "Please check the debug log."));
      }
      return;
    }
    m_sPreviousMove.clear();

    this->checkTowerWin(field);
    this->updatePlayers();
  } else {
    if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
      QMessageBox::information(NULL, tr("Information"),
                               tr("It is only allowed to place a "
                                  "stone on a free field."));
    } else {
      m_bScriptError = true;
      qWarning() << "CPU tried to set stone >>" << sMove;
      QMessageBox::warning(NULL, tr("Warning"),
                           tr("CPU script made an invalid move! "
                              "Please check the debug log."));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::moveTower(QPoint tower, QPoint moveTo, quint8 nStones) {
  QList<quint8> listStones(m_pBoard->getField(tower));
  if (0 == listStones.size()) {
    qWarning() << "Move tower size == 0! Tower:" << tower;
    if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
      QMessageBox::warning(NULL, tr("Warning"),
                           tr("Something went wrong!"));
    } else {
      m_bScriptError = true;
      QMessageBox::warning(NULL, tr("Warning"),
                           tr("CPU script made an invalid move! "
                              "Please check the debug log."));
    }
    return;
  }

  int nStonesToMove = 1;
  if (listStones.size() > 1 && 0 == nStones) {
    bool ok;
    nStonesToMove = QInputDialog::getInt(
                      NULL, tr("Move tower"),
                      tr("How many stones shall be moved:"),
                      1, 1, listStones.size(), 1, &ok);
    if (!ok) {
      return;
    }
  } else if (0 != nStones) {  // Call from CPU
    if (nStones > listStones.size()) {
      qWarning() << "Trying to move more stones than available! From:" << tower
                 << "Stones:" << nStones << "To:" << moveTo;
      if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
          (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
        QMessageBox::warning(NULL, tr("Warning"), tr("Something went wrong!"));
      } else {
        m_bScriptError = true;
        QMessageBox::warning(NULL, tr("Warning"),
                             tr("CPU script made an invalid move! "
                                "Please check the debug log."));
      }
      return;
    } else {
      nStonesToMove = nStones;
    }
  }

  // Debug print: E.g. "C4:3-D3" = move 3 stones from C4 to D3 (ASCII 65 = A)
  QString sMove(static_cast<char>(tower.x() + 65) +
                QString::number(tower.y() + 1) + ":" +
                QString::number(nStonesToMove) + "-" +
                static_cast<char>(moveTo.x() + 65) +
                QString::number(moveTo.y() + 1));

  if (m_pPlayer1->getIsActive()) {
    qDebug() << "P1 >>" << sMove;
    if (!m_pPlayer1->getIsHuman()) {
      m_pBoard->selectField(moveTo);
      m_pBoard->selectField(QPoint(-1, -1));
    }
  } else {
    qDebug() << "P2 >>" << sMove;
    if (!m_pPlayer2->getIsHuman()) {
      m_pBoard->selectField(moveTo);
      m_pBoard->selectField(QPoint(-1, -1));
    }
  }

  if (this->checkPreviousMoveReverted(sMove)) {
    if ((m_pPlayer1->getIsActive() && !m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && !m_pPlayer2->getIsHuman())) {
      m_bScriptError = true;
      qWarning() << "CPU tried to revert previous move.";
      QMessageBox::warning(NULL, tr("Warning"),
                           tr("CPU script made an invalid move! "
                              "Please check the debug log."));
    }
    QMessageBox::information(NULL, tr("Information"),
                             tr("It is not allowed to revert the "
                                "previous oppenents move directly!"));
    return;
  }

  // Check, if CPU made a valid move
  if (!m_pBoard->checkNeighbourhood(moveTo).contains(tower)) {
    qWarning() << "CPU tried to move a tower, which is not in the "
                  "neighbourhood of the selected tower.";
    m_bScriptError = true;
    QMessageBox::warning(NULL, tr("Warning"),
                         tr("CPU script made an invalid move! "
                            "Please check the debug log."));
    return;
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

void Game::checkTowerWin(QPoint field) {
  if (m_pBoard->getField(field).size() >= m_nMaxTowerHeight) {
    if (1 == m_pBoard->getField(field).last()) {
      m_pPlayer1->setWonTowers(m_pPlayer1->getWonTowers() + 1);
      qDebug() << "Player 1 conquered tower" <<
                  static_cast<char>(field.x() + 65) +
                  QString::number(field.y() + 1);
      if (m_pSettings->getWinTowers() != m_pPlayer1->getWonTowers()) {
        QMessageBox::information(NULL, tr("Information"),
                                 tr("%1 conquered a tower!")
                                 .arg(m_pPlayer1->getName()));
      }
    } else if (2 == m_pBoard->getField(field).last()) {
      m_pPlayer2->setWonTowers(m_pPlayer2->getWonTowers() + 1);
      qDebug() << "Player 2 conquered tower" <<
                  static_cast<char>(field.x() + 65) +
                  QString::number(field.y() + 1);
      if (m_pSettings->getWinTowers() != m_pPlayer2->getWonTowers()) {
        QMessageBox::information(NULL, tr("Information"),
                                 tr("%1 conquered a tower!")
                                 .arg(m_pPlayer2->getName()));
      }
    } else {
      qDebug() << Q_FUNC_INFO;
      qWarning() << "Last stone neither 1 nor 2!";
      qWarning() << "Field:" << field
                 << " -  Tower" << m_pBoard->getField(field);
      QMessageBox::warning(NULL, tr("Warning"),
                           tr("Something went wrong!"));
      return;
    }
    this->returnStones(field);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::returnStones(QPoint field) {
  QList<quint8> tower(m_pBoard->getField(field));
  quint8 stones(tower.count(1));
  m_pPlayer1->setStonesLeft(m_pPlayer1->getStonesLeft() + stones);
  stones = tower.count(2);
  m_pPlayer2->setStonesLeft(m_pPlayer2->getStonesLeft() + stones);

  // Clear field
  m_pBoard->removeStone(field, true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::updatePlayers(bool bInitial) {
  if (m_bScriptError) {
    emit setInteractive(false);
    return;
  }

  emit updateNameP1(m_pPlayer1->getName());
  emit updateNameP2(m_pPlayer2->getName());
  emit updateStonesP1(QString::number(m_pPlayer1->getStonesLeft()));
  emit updateStonesP2(QString::number(m_pPlayer2->getStonesLeft()));
  emit updateWonP1(QString::number(m_pPlayer1->getWonTowers()));
  emit updateWonP2(QString::number(m_pPlayer2->getWonTowers()));

  if (m_pSettings->getWinTowers() == m_pPlayer1->getWonTowers()) {
    qDebug() << "PLAYER 1 WON!";
    emit setInteractive(false);
    emit highlightActivePlayer(false, true);
    QMessageBox::information(NULL, tr("Information"), tr("%1 won the game!")
                             .arg(m_pPlayer1->getName()));
  } else if (m_pSettings->getWinTowers() == m_pPlayer2->getWonTowers()) {
    qDebug() << "PLAYER 2 WON!";
    emit setInteractive(false);
    emit highlightActivePlayer(false, false, true);
    QMessageBox::information(NULL, tr("Information"), tr("%1 won the game!")
                             .arg(m_pPlayer2->getName()));
  } else {
    if (!bInitial) {
      m_pPlayer1->setActive(!m_pPlayer1->getIsActive());
      m_pPlayer2->setActive(!m_pPlayer2->getIsActive());
    }
    emit highlightActivePlayer(m_pPlayer1->getIsActive());
    this->checkPossibleMoves();

    if ((m_pPlayer1->getIsActive() && !m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && !m_pPlayer2->getIsHuman())) {
      emit setInteractive(false);
      QTimer::singleShot(800, this, SLOT(delayCpu()));
    } else {
      emit setInteractive(true);
    }
  }

  m_pBoard->printDebugFields();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::delayCpu() {
  if (m_pPlayer1->getIsActive()) {
    emit makeMoveCpuP1(m_pBoard->getBoard(), m_pPlayer1->getCanMove());
  } else {
    emit makeMoveCpuP2(m_pBoard->getBoard(), m_pPlayer2->getCanMove());
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::checkPossibleMoves() {
  if (m_pPlayer1->getIsActive()) {
    m_pPlayer1->setCanMove(
          m_pBoard->findPossibleMoves(m_pPlayer1->getStonesLeft() > 0));
    if (0 != m_pPlayer1->getCanMove()) {
      return;
    }
  } else {
    m_pPlayer2->setCanMove(
          m_pBoard->findPossibleMoves(m_pPlayer2->getStonesLeft() > 0));
    if (0 != m_pPlayer2->getCanMove()) {
      return;
    }
  }

  if (0 == m_pPlayer1->getCanMove() && 0 == m_pPlayer2->getCanMove()) {
    emit setInteractive(false);
    qDebug() << "NO MOVES POSSIBLE ANYMORE!";
    QMessageBox::information(NULL, tr("Information"),
                             tr("No moves possible anymore.\n"
                                "Game ends in a tie!"));
  } else if (0 == m_pPlayer1->getCanMove()) {
    qDebug() << "PLAYER 1 HAS TO PASS!";
    QMessageBox::information(NULL, tr("Information"),
                             tr("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer1->getName()));
    this->updatePlayers();
  } else if (0 == m_pPlayer2->getCanMove()) {
    qDebug() << "PLAYER 2 HAS TO PASS!";
    QMessageBox::information(NULL, tr("Information"),
                             tr("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer2->getName()));
    this->updatePlayers();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Game::checkPreviousMoveReverted(const QString &sMove) {
  if (!m_sPreviousMove.isEmpty()) {
    QStringList sListPrev;
    QStringList sListCur;
    sListPrev = m_sPreviousMove.split(':');
    sListCur = sMove.split(':');

    if (2 == sListPrev.size() && 2 == sListCur.size()) {
      QString tmp(sListPrev[1]);
      sListPrev.removeLast();
      sListPrev << tmp.split('-');
      tmp = sListCur[1];
      sListCur.removeLast();
      sListCur << tmp.split('-');

      if (3 == sListPrev.size() && 3 == sListCur.size()) {
        if (sListPrev[0] == sListCur[2] &&
            sListPrev[1] == sListCur[1] &&
            sListPrev[2] == sListCur[0]) {
          return true;
        }
      } else {
        qDebug() << Q_FUNC_INFO;
        qWarning() << "Splitting 2 failed:" << m_sPreviousMove << sMove;
        QMessageBox::warning(NULL, tr("Warning"), tr("Something went wrong!"));
        return false;
      }
    } else {
      qDebug() << Q_FUNC_INFO;
      qWarning() << "Splitting 1 failed:" << m_sPreviousMove << sMove;
      QMessageBox::warning(NULL, tr("Warning"), tr("Something went wrong!"));
      return false;
    }
  }

  m_sPreviousMove = sMove;
  return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QJsonObject Game::loadGame(const QString &sFile) {
  QFile loadFile(sFile);

  if (!loadFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Couldn't open save file:" << sFile;
    return QJsonObject();
  }

  QByteArray saveData = loadFile.readAll();
  // QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
  QJsonDocument loadDoc(QJsonDocument::fromBinaryData(saveData));
  return loadDoc.object();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Game::saveGame(const QString &sFile) {
  QFile saveFile(sFile);
  QJsonArray tower;
  QVariantList vartower;
  QJsonArray jsBoard;
  QList<QList<QList<quint8> > > board(m_pBoard->getBoard());

  if (!saveFile.open(QIODevice::WriteOnly)) {
    qWarning() << "Couldn't open save file:" << sFile;
    return false;
  }

  // Convert board to json array
  for (int nRow = 0; nRow < m_nNumOfFields; nRow++) {
    QJsonArray line;
    for (int nCol = 0; nCol < m_nNumOfFields; nCol++) {
      vartower.clear();
      foreach (quint8 n, board[nRow][nCol]) {
        vartower << n;
      }
      tower = QJsonArray::fromVariantList(vartower);
      line.append(tower);
    }
    jsBoard.append(line);
  }

  QJsonObject jsonObj;
  jsonObj[QStringLiteral("Name1")] = m_pPlayer1->getName();
  jsonObj[QStringLiteral("Name2")] = m_pPlayer2->getName();
  jsonObj[QStringLiteral("Won1")] = m_pPlayer1->getWonTowers();
  jsonObj[QStringLiteral("Won2")] = m_pPlayer2->getWonTowers();
  jsonObj[QStringLiteral("HumanCpu1")] =
      m_pPlayer1->getIsHuman() ? QStringLiteral("Human") : m_sJsFileP1;
  jsonObj[QStringLiteral("HumanCpu2")] =
      m_pPlayer2->getIsHuman() ? QStringLiteral("Human") : m_sJsFileP2;
  jsonObj[QStringLiteral("Current")] = m_pPlayer1->getIsActive() ? 1 : 2;
  jsonObj[QStringLiteral("Board")] = jsBoard;

  QJsonDocument jsDoc(jsonObj);
  // if (-1 == saveFile.write(jsDoc.toJson())) {
  if (-1 == saveFile.write(jsDoc.toBinaryData())) {
    return false;
  }
  return true;
}
