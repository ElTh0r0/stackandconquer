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
#include "./settings.h"

Game::Game(Settings *pSettings, const QStringList &sListFiles)
  : m_pSettings(pSettings),
    m_pBoard(nullptr),
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

  connect(m_pBoard, &Board::actionPlayer, this, &Game::makeMove);

  if ("Human" != sP1HumanCpu) {
    m_sJsFileP1 = sP1HumanCpu;
  }

  if ("Human" != sP2HumanCpu) {
    m_sJsFileP2 = sP2HumanCpu;
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

  m_pPlayer1 = new Player((1 == nStartPlayer), 1, sName1,
                          m_pBoard->getMaxPlayerStones(), m_sJsFileP1);
  if (!m_pPlayer1->isHuman()) {
    connect(m_pPlayer1, &Player::actionCPU, this, &Game::makeMove);
    connect(m_pPlayer1, &Player::scriptError, this, &Game::caughtScriptError);
  }
  m_pPlayer2 = new Player((2 == nStartPlayer), 2, sName2,
                          m_pBoard->getMaxPlayerStones(), m_sJsFileP2);
  if (!m_pPlayer2->isHuman()) {
    connect(m_pPlayer2, &Player::actionCPU, this, &Game::makeMove);
    connect(m_pPlayer2, &Player::scriptError, this, &Game::caughtScriptError);
  }
  m_pPlayer1->setStonesLeft(nStonesLeftP1);
  m_pPlayer1->setWonTowers(nWonP1);
  m_pPlayer2->setStonesLeft(nStonesLeftP2);
  m_pPlayer2->setWonTowers(nWonP2);

  m_previousMove.clear();
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

auto Game::initCpu() -> bool {
  // TODO(x): Rewrite for > 2 players
  if (!m_pPlayer1->isHuman()) {
    if (!m_pPlayer1->initCPU(m_pBoard->getBoadDimensions(), m_nMaxTowerHeight,
                             m_pBoard->getOut(), m_pBoard->getPad())) {
      return false;
    }
  }
  if (!m_pPlayer2->isHuman()) {
    if (!m_pPlayer2->initCPU(m_pBoard->getBoadDimensions(), m_nMaxTowerHeight,
                             m_pBoard->getOut(), m_pBoard->getPad())) {
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

void Game::makeMove(QJsonArray move) {
  if (3 == move.size()) {
    bool bDebug(false);
    if (-999 == move.at(0).toInt()) {
      bDebug = true;
      move[0] = -1;
    }

    bool bSetStone = (-1 == move.at(0).toInt() && 1 == move.at(1).toInt());

    // TODO(x): Rewrite for > 2 players
    bool bValidMove(false);
    bool bIsHuman(false);
    if (m_pPlayer1->isActive()) {
      bValidMove = this->checkMoveIsValid(m_pPlayer1->getLegalMoves(), move);
      bIsHuman = m_pPlayer1->isHuman();
    }
    if (m_pPlayer2->isActive()) {
      bValidMove = this->checkMoveIsValid(m_pPlayer2->getLegalMoves(), move);
      bIsHuman = m_pPlayer2->isHuman();
    }
    if (bDebug) {
      bValidMove = true;
    }

    if (!bValidMove) {
      if (!bIsHuman) {
        m_bScriptError = true;
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("CPU script made an invalid move! "
                                "Please check the debug log."));
      } else {
        if (bSetStone) {
          QMessageBox::information(nullptr, tr("Information"),
                                   tr("No stones left! Please move a tower."));
        } else {
          qWarning() << "Invalid move!" << move;
          QMessageBox::information(nullptr, tr("Warning"),
                                   tr("Invalid move!"));
        }
      }
      return;
    }

    if (bSetStone) {
      this->setStone(move.at(2).toInt(), bDebug);
    } else {
      this->moveTower(move.at(0).toInt(),
                      move.at(1).toInt(),
                      move.at(2).toInt());
    }
  } else {
    qWarning() << "Invalid move!" << move;
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::checkMoveIsValid(const QJsonDocument &legalMoves,
                            const QJsonArray &move) -> bool {
  QJsonArray allMoves(legalMoves.array());
  if (allMoves.contains(move)) {
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::setStone(const int nIndex, const bool bDebug) {
  // TODO(x): Rewrite for > 2 players
  // Below checks never should fail, due to comparisson with legal moves list...
  if (m_pBoard->getField(nIndex).isEmpty() || bDebug) {
    if (m_pPlayer1->isActive() && m_pPlayer1->getStonesLeft() > 0) {
      m_pPlayer1->setStonesLeft(m_pPlayer1->getStonesLeft() - 1);
      m_pBoard->addStone(nIndex, 1);
      qDebug() << "P" + m_pPlayer1->getID() + " >> " +
                  m_pBoard->getStringCoordFromIndex(nIndex);
    } else if (m_pPlayer2->isActive() && m_pPlayer2->getStonesLeft() > 0) {
      m_pPlayer2->setStonesLeft(m_pPlayer2->getStonesLeft() - 1);
      m_pBoard->addStone(nIndex, 2);
      qDebug() << "P" + m_pPlayer2->getID() + " >> " +
                  m_pBoard->getStringCoordFromIndex(nIndex);
    } else {
      qWarning() << "Invalid move! Set stone:" << nIndex;
      QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
      return;
    }

    m_previousMove.clear();
    this->checkTowerWin(nIndex);
    this->updatePlayers();
  } else {
    qWarning() << "Invalid move! Set stone:" << nIndex;
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::moveTower(const int nFrom, const quint8 nStones, const int nTo) {
  QList<int> listStones;
  for (auto ch : m_pBoard->getField(nFrom)) {
    listStones.append(ch.digitValue());
  }

  // Below checks never should fail, due to comparisson with legal moves list...
  if (listStones.contains(-1)) {
    qWarning() << "Tower contains invalid stone!" << listStones;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("Something went wrong!"));
    return;
  }
  if (listStones.isEmpty()) {
    qWarning() << "Move tower size == 0! Tower:" << nFrom;
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("Something went wrong!"));
    return;
  }
  if (nStones > listStones.size()) {
    qWarning() << "Trying to move more stones than available! From:" << nFrom
               << "Stones:" << nStones << "To:" << nTo;
      QMessageBox::warning(nullptr, tr("Warning"),
                           tr("Something went wrong!"));
    return;
  }
  if (!m_pBoard->checkNeighbourhood(nTo).contains(nFrom)) {
    qWarning() << "CPU tried to move a tower, which is not in the "
                  "neighbourhood of the selected tower.";
    m_bScriptError = true;
    QMessageBox::warning(nullptr, tr("Warning"),
                         tr("CPU script made an invalid move! "
                            "Please check the debug log."));
    return;
  }

  // Debug print: E.g. "C4:3-D3" = move 3 stones from C4 to D3 (ASCII 65 = A)
  QString sMove(m_pBoard->getStringCoordFromIndex(nFrom) + ":" +
                QString::number(nStones) + "-" +
                m_pBoard->getStringCoordFromIndex(nTo));

  if (m_pPlayer1->isActive()) {
    qDebug() << "P" + m_pPlayer1->getID() + " >> " + sMove;
    if (!m_pPlayer1->isHuman()) {
      m_pBoard->selectIndexField(nTo);
      m_pBoard->selectIndexField(-1);
    }
  } else {
    qDebug() << "P" + m_pPlayer2->getID() + " >> " + sMove;
    if (!m_pPlayer2->isHuman()) {
      m_pBoard->selectIndexField(nTo);
      m_pBoard->selectIndexField(-1);
    }
  }

  for (int i = 0; i < nStones; i++) {
    m_pBoard->removeStone(nFrom);  // Remove is in the wrong order, nevermind!
    m_pBoard->addStone(nTo,
                       listStones[listStones.size() - nStones + i]);
  }

  m_previousMove.clear();
  m_previousMove << nFrom << nStones << nTo;

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
      qDebug() << "Player " + m_pPlayer1->getID() + " conquered tower " +
                  m_pBoard->getStringCoordFromIndex(nIndex);
      if (m_nWinTowers != m_pPlayer1->getWonTowers()) {
        QMessageBox::information(nullptr, tr("Information"),
                                 tr("%1 conquered a tower!")
                                 .arg(m_pPlayer1->getName()));
      }
    } else if (2 == m_pBoard->getField(nIndex).rightRef(1).toInt()) {
      m_pPlayer2->setWonTowers(m_pPlayer2->getWonTowers() + 1);
      qDebug() << "Player " + m_pPlayer2->getID() + " conquered tower " +
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
  // TODO(x): Rewrite for > 2 player - add "active player" variable + if is CPU!
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
    qDebug() << "PLAYER " + m_pPlayer1->getID() + " WON!";
    emit setInteractive(false);
    emit highlightActivePlayer(false, true);
    QMessageBox::information(nullptr, tr("Information"), tr("%1 won the game!")
                             .arg(m_pPlayer1->getName()));
  } else if (m_nWinTowers == m_pPlayer2->getWonTowers()) {
    qDebug() << "PLAYER " + m_pPlayer2->getID() + " WON!";
    emit setInteractive(false);
    emit highlightActivePlayer(false, false, true);
    QMessageBox::information(nullptr, tr("Information"), tr("%1 won the game!")
                             .arg(m_pPlayer2->getName()));
  } else {
    if (!bInitial) {
      m_pPlayer1->setActive(!m_pPlayer1->isActive());
      m_pPlayer2->setActive(!m_pPlayer2->isActive());
    }
    emit highlightActivePlayer(m_pPlayer1->isActive());
    if (!this->checkPossibleMoves()) {
      m_pBoard->printDebugFields();
      return;
    }

    if ((m_pPlayer1->isActive() && !m_pPlayer1->isHuman()) ||
        (m_pPlayer2->isActive() && !m_pPlayer2->isHuman())) {
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
  if (m_pPlayer1->isActive()) {
    m_pPlayer1->callCpu(m_pBoard->getBoard(), m_pPlayer1->getLegalMoves());
  } else {
    m_pPlayer2->callCpu(m_pBoard->getBoard(), m_pPlayer2->getLegalMoves());
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::checkPossibleMoves() -> bool {
  m_pPlayer1->setLegalMoves(m_pBoard->getLegalMoves(
                              m_pPlayer1->getStonesLeft() > 0,
                              m_previousMove));
  m_pPlayer2->setLegalMoves(m_pBoard->getLegalMoves(
                              m_pPlayer2->getStonesLeft() > 0,
                              m_previousMove));
  // TODO(x): Rewrite for > 2 players
  if (m_pPlayer1->isActive() && m_pPlayer1->canMove()) {
    return true;
  }
  if (m_pPlayer2->isActive() && m_pPlayer2->canMove()) {
    return true;
  }

  if (!m_pPlayer1->canMove() && !m_pPlayer2->canMove()) {
    emit setInteractive(false);
    qDebug() << "NO MOVES POSSIBLE ANYMORE!";
    QMessageBox::information(nullptr, tr("Information"),
                             tr("No moves possible anymore.\n"
                                "Game ends in a tie!"));
    return false;
  } else if (!m_pPlayer1->canMove()) {
    qDebug() << "PLAYER " + m_pPlayer1->getID() + " HAS TO PASS!";
    QMessageBox::information(nullptr, tr("Information"),
                             tr("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer1->getName()));
    this->updatePlayers();
  } else if (!m_pPlayer2->canMove()) {
    qDebug() << "PLAYER " + m_pPlayer2->getID() + " HAS TO PASS!";
    QMessageBox::information(nullptr, tr("Information"),
                             tr("No move possible!\n%1 has to pass.")
                             .arg(m_pPlayer2->getName()));
    this->updatePlayers();
  }
  return true;
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
      m_pPlayer1->isHuman() ? QStringLiteral("Human") : m_sJsFileP1;
  jsonObj[QStringLiteral("HumanCpu2")] =
      m_pPlayer2->isHuman() ? QStringLiteral("Human") : m_sJsFileP2;
  jsonObj[QStringLiteral("Current")] = m_pPlayer1->isActive() ? 1 : 2;
  jsonObj[QStringLiteral("Board")] = jsBoard;
  jsonObj[QStringLiteral("BoardFile")] = m_sBoardFile;
  jsonObj[QStringLiteral("BoardFileRelative")] = sRelativeDir;
  jsonObj[QStringLiteral("BoardColumns")] = m_pBoard->getBoadDimensions().x();
  jsonObj[QStringLiteral("BoardRows")] = m_pBoard->getBoadDimensions().y();
  QJsonDocument jsDoc(jsonObj);

  return (-1 != saveFile.write(jsDoc.toJson().toBase64()));
}
