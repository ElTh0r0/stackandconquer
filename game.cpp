/**
 * \file game.cpp
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
 * Main game engine (object creation etc.).
 */

#include "./game.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMessageBox>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#else
#include <QTime>  // Seed qsrand
#endif

#include "./board.h"
#include "./player.h"
#include "./opponentjs.h"
#include "./settings.h"

Game::Game(Settings *pSettings, const QStringList &sListFiles)
  : m_pSettings(pSettings),
    m_pBoard(nullptr),
    m_jsCpuP1(nullptr),
    m_jsCpuP2(nullptr),
    m_pPlayer1(nullptr),
    m_pPlayer2(nullptr),
    m_sJsFileP1(QLatin1String("")),
    m_sJsFileP2(QLatin1String("")),
    m_nMaxTowerHeight(5),
    m_nGridSize(70),
    m_nWinTowers(pSettings->getWinTowers()),
    m_bScriptError(false) {
  qDebug() << "Starting new game" << sListFiles;

  quint8 nNumOfPlayers(0);  // TODO(x): Rewrite for > 2 players
  QString sP1HumanCpu(QLatin1String(""));
  QString sName1(QStringLiteral("P1"));
  QString sP2HumanCpu(QLatin1String(""));
  QString sName2(QStringLiteral("P2"));
  quint8 nStartPlayer(0);
  quint8 nStonesLeftP1(0);
  quint8 nStonesLeftP2(0);
  quint8 nWonP1(0);
  quint8 nWonP2(0);

  if (1 == sListFiles.size()) {
    if (sListFiles[0].endsWith(QStringLiteral(".stackboard"),
                               Qt::CaseInsensitive)) {  // Start with default
      m_sBoardFile = sListFiles[0];
      sP1HumanCpu = m_pSettings->getPlayerHumanCpu(1);
      sName1 = m_pSettings->getPlayerName(1);
      sP2HumanCpu = m_pSettings->getPlayerHumanCpu(2);
      sName2 = m_pSettings->getPlayerName(2);
      nStartPlayer = m_pSettings->getStartPlayer();
      nNumOfPlayers = m_pSettings->getNumOfPlayers();
    } else if (sListFiles[0].endsWith(QStringLiteral(".stacksav"),
                               Qt::CaseInsensitive)) {  // Load
      QJsonObject jsonObj(Game::loadGame(sListFiles[0]));
      if (jsonObj.isEmpty()) {
        qWarning() << "Save file is empty!";
        QMessageBox::critical(nullptr, tr("Warning"),
                             tr("Error while opening save game."));
        exit(-1);
      }

      // TODO(x): Rewrite for > 2 players
      nNumOfPlayers = static_cast<quint8>(
                        jsonObj[QStringLiteral("NumOfPlayers")].toInt());
      sP1HumanCpu = jsonObj[QStringLiteral("HumanCpu1")].toString().trimmed();
      sName1 = jsonObj[QStringLiteral("Name1")].toString().trimmed();
      nWonP1 = static_cast<quint8>(jsonObj[QStringLiteral("Won1")].toInt());
      nStonesLeftP1 = static_cast<quint8>(
                        jsonObj[QStringLiteral("StonesLeft1")].toInt());
      sP2HumanCpu = jsonObj[QStringLiteral("HumanCpu2")].toString().trimmed();
      sName2 = jsonObj[QStringLiteral("Name2")].toString().trimmed();
      nWonP2 = static_cast<quint8>(jsonObj[QStringLiteral("Won2")].toInt());
      nStonesLeftP2 = static_cast<quint8>(
                        jsonObj[QStringLiteral("StonesLeft2")].toInt());
      nStartPlayer = static_cast<quint8>(
                       jsonObj[QStringLiteral("Current")].toInt());
      m_nWinTowers = static_cast<quint8>(
                       jsonObj[QStringLiteral("WinTowers")].toInt());
      quint16 nBoardColumns =
          static_cast<quint16>(
            jsonObj[QStringLiteral("BoardColumns")].toInt(0));
      quint16 nBoardRows = static_cast<quint16>(
                             jsonObj[QStringLiteral("BoardRows")].toInt(0));

      if (sP1HumanCpu.isEmpty() || sP2HumanCpu.isEmpty() ||
          sName1.isEmpty() || sName2.isEmpty() ||
          0 == nBoardColumns || 0 == nBoardRows || 0 == nNumOfPlayers ||
          0 == nStartPlayer || 0 == m_nWinTowers) {
        qWarning() << "Save game contains invalid data:"
                   << "sP1HumanCpu / sP2HumanCpu / sName1 / sName2 /"
                   << "nBoardColumns / nBoardRows / nNumOfPlayers /"
                      "nStartPlayer / m_nWinTowers is empty.";
        QMessageBox::critical(nullptr, tr("Warning"),
                              tr("Save game contains invalid data."));
        exit(-1);
      }

      m_sBoardFile = jsonObj[QStringLiteral("BoardFile")].toString().trimmed();
      QString sRelBoard =
          jsonObj[QStringLiteral("BoardFileRelative")].toString().trimmed();
      if (m_sBoardFile.isEmpty() || !QFile::exists(m_sBoardFile)) {
        if (!sRelBoard.isEmpty() &&
            QFile::exists(qApp->applicationDirPath() + "/" + sRelBoard)) {
          m_sBoardFile = qApp->applicationDirPath() + "/" + sRelBoard;
        } else {
          qWarning() << "Save game contains invalid data - board not found!";
          QMessageBox::warning(nullptr, qApp->applicationName(),
                               tr("Save game contains invalid data."));
          exit(-1);
        }
      }
      qDebug() << "Loading save game board:" << m_sBoardFile;

      QJsonArray jsBoard = jsonObj[QStringLiteral("Board")].toArray();
      m_pBoard = new Board(m_sBoardFile, m_nGridSize, m_nMaxTowerHeight,
                           nNumOfPlayers, m_pSettings);
      if (!m_pBoard->setupSavegame(jsBoard)) {
        qWarning() << "Save game contains invalid data - board not found!";
        QMessageBox::warning(nullptr, qApp->applicationName(),
                             tr("Save game contains invalid data."));
        exit(-1);
      }
    } else if (sListFiles[0].endsWith(QStringLiteral(".js"),
                                      Qt::CaseInsensitive)) {  // 1 CPU
      sP1HumanCpu = QStringLiteral("Human");
      sName1 = m_pSettings->getPlayerName(1);
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
  } else {
    qWarning() << "Invalid start up!";
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
  }

  // No save game: Start empty board with default values
  if (nullptr == m_pBoard) {
    m_pBoard = new Board(m_sBoardFile, m_nGridSize, m_nMaxTowerHeight,
                         nNumOfPlayers, m_pSettings);
    nStonesLeftP1 = m_pBoard->getMaxPlayerStones();
    nStonesLeftP2 = m_pBoard->getMaxPlayerStones();
  }

  connect(m_pBoard, &Board::setStone, this, &Game::setStone);
  connect(m_pBoard, &Board::moveTower, this, &Game::moveTower);

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
  if (0 == nStartPlayer) {  // Random
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    nStartPlayer = QRandomGenerator::global()->bounded(
                     1, m_pSettings->getNumOfPlayers() + 1);
#else
    qsrand(static_cast<uint>(QTime::currentTime().msec()));  // Seed
    nStartPlayer = qrand() % m_pSettings->getNumOfPlayers() + 1;
#endif
  }

  m_pPlayer1 = new Player((1 == nStartPlayer), bP1IsHuman,
                          sName1, m_pBoard->getMaxPlayerStones());
  m_pPlayer2 = new Player((2 == nStartPlayer), bP2IsHuman,
                          sName2, m_pBoard->getMaxPlayerStones());
  m_pPlayer1->setStonesLeft(nStonesLeftP1);
  m_pPlayer1->setWonTowers(nWonP1);
  m_pPlayer2->setStonesLeft(nStonesLeftP2);
  m_pPlayer2->setWonTowers(nWonP2);

  m_sPreviousMove.clear();
}

Game::~Game() {
  delete m_pBoard;
  m_pBoard = nullptr;
  delete m_pPlayer1;
  m_pPlayer1 = nullptr;
  delete m_pPlayer2;
  m_pPlayer2 = nullptr;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::createCPU1() {
  m_jsCpuP1 = new OpponentJS(1, m_pBoard->getBoadDimensions(),
                             m_nMaxTowerHeight, m_pBoard->getOut(),
                             m_pBoard->getPad());
  connect(this, &Game::makeMoveCpuP1, m_jsCpuP1, &OpponentJS::makeMoveCpu);
  connect(m_jsCpuP1, &OpponentJS::setStone, this, &Game::setStone);
  connect(m_jsCpuP1, &OpponentJS::moveTower, this, &Game::moveTower);
  connect(m_jsCpuP1, &OpponentJS::scriptError, this, &Game::caughtScriptError);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::createCPU2() {
  m_jsCpuP2 = new OpponentJS(2, m_pBoard->getBoadDimensions(),
                             m_nMaxTowerHeight, m_pBoard->getOut(),
                             m_pBoard->getPad());
  connect(this, &Game::makeMoveCpuP2, m_jsCpuP2, &OpponentJS::makeMoveCpu);
  connect(m_jsCpuP2, &OpponentJS::setStone, this, &Game::setStone);
  connect(m_jsCpuP2, &OpponentJS::moveTower, this, &Game::moveTower);
  connect(m_jsCpuP2, &OpponentJS::scriptError, this, &Game::caughtScriptError);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::initCpu() -> bool {
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

auto Game::getScene() const -> QGraphicsScene* {
  return m_pBoard;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::setStone(int nIndex, bool bDebug) {
  // TODO(x): Rewrite for > 2 players
  if (m_pBoard->getField(nIndex).isEmpty() || bDebug) {
    if (m_pPlayer1->getIsActive() && m_pPlayer1->getStonesLeft() > 0) {
      m_pPlayer1->setStonesLeft(m_pPlayer1->getStonesLeft() - 1);
      m_pBoard->addStone(nIndex, 1);
      qDebug() << "P1 >>" << m_pBoard->getStringCoordFromIndex(nIndex);
    } else if (m_pPlayer2->getIsActive() && m_pPlayer2->getStonesLeft() > 0) {
      m_pPlayer2->setStonesLeft(m_pPlayer2->getStonesLeft() - 1);
      m_pBoard->addStone(nIndex, 2);
      qDebug() << "P2 >>" << m_pBoard->getStringCoordFromIndex(nIndex);
    } else {
      if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
          (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
        QMessageBox::information(nullptr, tr("Information"),
                                 tr("No stones left! Please move a tower."));
      } else {
        m_bScriptError = true;
        qWarning() << "CPU tried to set stone, but no stones left!";
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("CPU script made an invalid move! "
                                "Please check the debug log."));
      }
      return;
    }
    m_sPreviousMove.clear();

    this->checkTowerWin(nIndex);
    this->updatePlayers();
  } else {
    if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
      QMessageBox::information(nullptr, tr("Information"),
                               tr("It is only allowed to place a "
                                  "stone on a free field."));
    } else {
      m_bScriptError = true;
      qWarning() << "CPU tried to set stone >>" <<
                    m_pBoard->getStringCoordFromIndex(nIndex);
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("CPU script made an invalid move! "
                              "Please check the debug log."));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::moveTower(int nFrom, quint8 nStones, int nTo) {
  QList<int> listStones;
  for (auto ch : m_pBoard->getField(nFrom)) {
    listStones.append(ch.digitValue());
  }
  if (listStones.contains(-1)) {
    qWarning() << "Tower contains invalid stone!" << listStones;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Something went wrong!"));
    return;
  }

  if (listStones.isEmpty()) {
    qWarning() << "Move tower size == 0! Tower:" << nFrom;
    if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("Something went wrong!"));
    } else {
      m_bScriptError = true;
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("CPU script made an invalid move! "
                              "Please check the debug log."));
    }
    return;
  }

  int nStonesToMove = 1;
  if (listStones.size() > 1 && 0 == nStones) {
    bool ok;
    nStonesToMove = QInputDialog::getInt(
                      nullptr, tr("Move tower"),
                      tr("How many stones shall be moved:"),
                      1, 1, listStones.size(), 1, &ok);
    if (!ok) {
      return;
    }
  } else if (0 != nStones) {  // Call from CPU
    if (nStones > listStones.size()) {
      qWarning() << "Trying to move more stones than available! From:" << nFrom
                 << "Stones:" << nStones << "To:" << nTo;
      if ((m_pPlayer1->getIsActive() && m_pPlayer1->getIsHuman()) ||
          (m_pPlayer2->getIsActive() && m_pPlayer2->getIsHuman())) {
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Something went wrong!"));
      } else {
        m_bScriptError = true;
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("CPU script made an invalid move! "
                                "Please check the debug log."));
      }
      return;
    }
    nStonesToMove = nStones;
  }

  // Debug print: E.g. "C4:3-D3" = move 3 stones from C4 to D3 (ASCII 65 = A)
  QString sMove(m_pBoard->getStringCoordFromIndex(nFrom) + ":" +
                QString::number(nStonesToMove) + "-" +
                m_pBoard->getStringCoordFromIndex(nTo));

  if (m_pPlayer1->getIsActive()) {
    qDebug() << "P1 >>" << sMove;
    if (!m_pPlayer1->getIsHuman()) {
      m_pBoard->selectIndexField(nTo);
      m_pBoard->selectIndexField(-1);
    }
  } else {
    qDebug() << "P2 >>" << sMove;
    if (!m_pPlayer2->getIsHuman()) {
      m_pBoard->selectIndexField(nTo);
      m_pBoard->selectIndexField(-1);
    }
  }

  if (this->checkPreviousMoveReverted(sMove)) {
    if ((m_pPlayer1->getIsActive() && !m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && !m_pPlayer2->getIsHuman())) {
      m_bScriptError = true;
      qWarning() << "CPU tried to revert previous move.";
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("CPU script made an invalid move! "
                              "Please check the debug log."));
    }
    QMessageBox::information(nullptr, tr("Information"),
                             tr("It is not allowed to revert the "
                                "previous oppenents move directly!"));
    return;
  }

  // Check, if CPU made a valid move
  if (!m_pBoard->checkNeighbourhood(nTo).contains(nFrom)) {
    qWarning() << "CPU tried to move a tower, which is not in the "
                  "neighbourhood of the selected tower.";
    m_bScriptError = true;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("CPU script made an invalid move! "
                            "Please check the debug log."));
    return;
  }

  for (int i = 0; i < nStonesToMove; i++) {
    m_pBoard->removeStone(nFrom);  // Remove is in the wrong order, nevermind!
    m_pBoard->addStone(nTo,
                       listStones[listStones.size() - nStonesToMove + i]);
  }

  this->checkTowerWin(nTo);
  this->updatePlayers();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::checkTowerWin(const int nIndex) {
  // TODO(x): Rewrite for > 2 players
  if (m_pBoard->getField(nIndex).size() >= m_nMaxTowerHeight) {
    if (1 == m_pBoard->getField(nIndex).rightRef(1).toInt()) {
      m_pPlayer1->setWonTowers(m_pPlayer1->getWonTowers() + 1);
      qDebug() << "Player 1 conquered tower" <<
                  m_pBoard->getStringCoordFromIndex(nIndex);
      if (m_nWinTowers != m_pPlayer1->getWonTowers()) {
        QMessageBox::information(nullptr, tr("Information"),
                                 tr("%1 conquered a tower!")
                                 .arg(m_pPlayer1->getName()));
      }
    } else if (2 == m_pBoard->getField(nIndex).rightRef(1).toInt()) {
      m_pPlayer2->setWonTowers(m_pPlayer2->getWonTowers() + 1);
      qDebug() << "Player 2 conquered tower" <<
                  m_pBoard->getStringCoordFromIndex(nIndex);
      if (m_nWinTowers != m_pPlayer2->getWonTowers()) {
        QMessageBox::information(nullptr, tr("Information"),
                                 tr("%1 conquered a tower!")
                                 .arg(m_pPlayer2->getName()));
      }
    } else {
      qDebug() << Q_FUNC_INFO;
      qWarning() << "Last stone neither 1 nor 2!";
      qWarning() << "Field:" << nIndex
                 << " -  Tower" << m_pBoard->getField(nIndex);
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("Something went wrong!"));
      return;
    }
    this->returnStones(nIndex);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::returnStones(const int nIndex) {
  // TODO(x): Rewrite for > 2 players
  QString tower(m_pBoard->getField(nIndex));
  quint8 stones(static_cast<quint8>(tower.count('1')));
  m_pPlayer1->setStonesLeft(m_pPlayer1->getStonesLeft() + stones);
  stones = static_cast<quint8>(tower.count('2'));
  m_pPlayer2->setStonesLeft(m_pPlayer2->getStonesLeft() + stones);

  // Clear field
  m_pBoard->removeStone(nIndex, true);
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

  if (m_nWinTowers == m_pPlayer1->getWonTowers()) {
    qDebug() << "PLAYER 1 WON!";
    emit setInteractive(false);
    emit highlightActivePlayer(false, true);
    QMessageBox::information(nullptr, tr("Information"), tr("%1 won the game!")
                             .arg(m_pPlayer1->getName()));
  } else if (m_nWinTowers == m_pPlayer2->getWonTowers()) {
    qDebug() << "PLAYER 2 WON!";
    emit setInteractive(false);
    emit highlightActivePlayer(false, false, true);
    QMessageBox::information(nullptr, tr("Information"), tr("%1 won the game!")
                             .arg(m_pPlayer2->getName()));
  } else {
    if (!bInitial) {
      m_pPlayer1->setActive(!m_pPlayer1->getIsActive());
      m_pPlayer2->setActive(!m_pPlayer2->getIsActive());
    }
    emit highlightActivePlayer(m_pPlayer1->getIsActive());
    if (!this->checkPossibleMoves()) {
      m_pBoard->printDebugFields();
      return;
    }

    if ((m_pPlayer1->getIsActive() && !m_pPlayer1->getIsHuman()) ||
        (m_pPlayer2->getIsActive() && !m_pPlayer2->getIsHuman())) {
      emit setInteractive(false);
      QTimer::singleShot(800, this, &Game::delayCpu);
    } else {
      emit setInteractive(true);
    }
  }

  m_pBoard->printDebugFields();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::delayCpu() {
  // TODO(x): Rewrite for > 2 players
  if (m_pPlayer1->getIsActive()) {
    emit makeMoveCpuP1(m_pBoard->getBoard(), m_pPlayer1->getCanMove());
  } else {
    emit makeMoveCpuP2(m_pBoard->getBoard(), m_pPlayer2->getCanMove());
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::checkPossibleMoves() -> bool {
  m_pPlayer1->setCanMove(
        m_pBoard->findPossibleMoves(m_pPlayer1->getStonesLeft() > 0));
  m_pPlayer2->setCanMove(
        m_pBoard->findPossibleMoves(m_pPlayer2->getStonesLeft() > 0));
  // TODO(x): Rewrite for > 2 players
  if (m_pPlayer1->getIsActive() && 0 != m_pPlayer1->getCanMove()) {
    return true;
  }
  if (m_pPlayer2->getIsActive() && 0 != m_pPlayer2->getCanMove()) {
    return true;
  }

  if (0 == m_pPlayer1->getCanMove() && 0 == m_pPlayer2->getCanMove()) {
    emit setInteractive(false);
    qDebug() << "NO MOVES POSSIBLE ANYMORE!";
    QMessageBox::information(nullptr, tr("Information"),
                             tr("No moves possible anymore.\n"
                                "Game ends in a tie!"));
    return false;
  } else if (0 == m_pPlayer1->getCanMove()) {
    qDebug() << "PLAYER 1 HAS TO PASS!";
    QMessageBox::information(nullptr, tr("Information"),
                             tr("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer1->getName()));
    this->updatePlayers();
  } else if (0 == m_pPlayer2->getCanMove()) {
    qDebug() << "PLAYER 2 HAS TO PASS!";
    QMessageBox::information(nullptr, tr("Information"),
                             tr("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer2->getName()));
    this->updatePlayers();
  }
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::checkPreviousMoveReverted(const QString &sMove) -> bool {
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
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Something went wrong!"));
        return false;
      }
    } else {
      qDebug() << Q_FUNC_INFO;
      qWarning() << "Splitting 1 failed:" << m_sPreviousMove << sMove;
      QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
      return false;
    }
  }

  m_sPreviousMove = sMove;
  return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::loadGame(const QString &sFile) -> QJsonObject {
  QFile loadFile(sFile);

  if (!loadFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Couldn't open save file:" << sFile;
    return QJsonObject();
  }

  QByteArray saveData = QByteArray::fromBase64(loadFile.readAll());
  QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
  return loadDoc.object();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::saveGame(const QString &sFile) -> bool {
  QFile saveFile(sFile);
  QJsonArray jsBoard(m_pBoard->getBoard());

  if (!saveFile.open(QIODevice::WriteOnly)) {
    qWarning() << "Couldn't open save file:" << sFile;
    return false;
  }

  // Save relative board folder, needed e.g. for AppImage
  QDir dir(qApp->applicationDirPath());
  QString sRelativeDir;
  sRelativeDir = dir.relativeFilePath(m_sBoardFile);

  // TODO(x): Rewrite for > 2 players
  QJsonObject jsonObj;
  jsonObj[QStringLiteral("NumOfPlayers")] = 2;
  jsonObj[QStringLiteral("Name1")] = m_pPlayer1->getName();
  jsonObj[QStringLiteral("Name2")] = m_pPlayer2->getName();
  jsonObj[QStringLiteral("Won1")] = m_pPlayer1->getWonTowers();
  jsonObj[QStringLiteral("Won2")] = m_pPlayer2->getWonTowers();
  jsonObj[QStringLiteral("WinTowers")] = m_nWinTowers;
  jsonObj[QStringLiteral("StonesLeft1")] = m_pPlayer1->getStonesLeft();
  jsonObj[QStringLiteral("StonesLeft2")] = m_pPlayer2->getStonesLeft();
  jsonObj[QStringLiteral("HumanCpu1")] =
      m_pPlayer1->getIsHuman() ? QStringLiteral("Human") : m_sJsFileP1;
  jsonObj[QStringLiteral("HumanCpu2")] =
      m_pPlayer2->getIsHuman() ? QStringLiteral("Human") : m_sJsFileP2;
  jsonObj[QStringLiteral("Current")] = m_pPlayer1->getIsActive() ? 1 : 2;
  jsonObj[QStringLiteral("Board")] = jsBoard;
  jsonObj[QStringLiteral("BoardFile")] = m_sBoardFile;
  jsonObj[QStringLiteral("BoardFileRelative")] = sRelativeDir;
  jsonObj[QStringLiteral("BoardColumns")] = m_pBoard->getBoadDimensions().x();
  jsonObj[QStringLiteral("BoardRows")] = m_pBoard->getBoadDimensions().y();
  QJsonDocument jsDoc(jsonObj);

  return (-1 != saveFile.write(jsDoc.toJson().toBase64()));
}
