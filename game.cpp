/**
 * \file game.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-present Thorsten Roth
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

Game::Game(QWidget *pParent, Settings *pSettings, const QString &sIN,
           const QString &sOUT, QObject *pParentObj)
    : m_pParent(pParent),
      m_pSettings(pSettings),
      m_sIN(sIN),
      m_sOUT(sOUT),
      m_pBoard(nullptr),
      m_sBoardFile(pSettings->getBoardFile()),
      m_nNumOfPlayers(pSettings->getNumOfPlayers()),
      m_nMaxTowerHeight(5),
      m_nTowersToWin(pSettings->getTowersToWin()),
      m_bScriptError(false) {
  Q_UNUSED(pParentObj)

  // Dummy init; will be set in createGame()
  activePlayer.ID = 1;
  activePlayer.isHuman = true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

Game::~Game() {
  delete m_pBoard;
  m_pBoard = nullptr;
  for (int i = 0; i < m_pPlayers.size(); i++) {
    delete m_pPlayers[i];
    m_pPlayers[i] = nullptr;
  }
  m_pPlayers.clear();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::createGame(const QString &sSavegame) -> bool {
  qDebug() << "Starting new game" << sSavegame;

  quint8 nStartPlayer(m_pSettings->getStartPlayer());
  QJsonArray Won;
  QJsonArray StonesLeft;
  QJsonArray CpuScript;

  if (!sSavegame.isEmpty()) {  // Savegame
    QJsonObject jsonObj(Game::loadGame(sSavegame));
    if (jsonObj.isEmpty()) {
      qWarning() << "Save file is empty!";
      QMessageBox::critical(m_pParent, tr("Warning"),
                            tr("Error while opening save game."));
      return false;
    }

    nStartPlayer =
        static_cast<quint8>(jsonObj[QStringLiteral("Current")].toInt());
    m_nTowersToWin =
        static_cast<quint8>(jsonObj[QStringLiteral("WinTowers")].toInt());
    if (0 == nStartPlayer || 0 == m_nTowersToWin) {
      qWarning() << "Save game contains invalid data:"
                 << "nStartPlayer or m_nWinTowers empty";
      QMessageBox::critical(m_pParent, tr("Warning"),
                            tr("Save game contains invalid data."));
      return false;
    }

    m_nNumOfPlayers =
        static_cast<quint8>(jsonObj[QStringLiteral("NumOfPlayers")].toInt());
    if (m_nNumOfPlayers > m_pSettings->getMaxNumOfPlayers() ||
        nStartPlayer > m_nNumOfPlayers) {
      qWarning() << "Save game contains invalid data:"
                 << "NumOfPlayers > max number of players or "
                    "nStartPlayer > m_nNumOfPlayers";
      QMessageBox::critical(m_pParent, tr("Warning"),
                            tr("Save game contains invalid data."));
      return false;
    }
    CpuScript = jsonObj[QStringLiteral("CpuScript")].toArray();
    Won = jsonObj[QStringLiteral("Won")].toArray();
    StonesLeft = jsonObj[QStringLiteral("StonesLeft")].toArray();

    if (0 == m_nNumOfPlayers || CpuScript.size() != m_nNumOfPlayers ||
        Won.size() != m_nNumOfPlayers || StonesLeft.size() != m_nNumOfPlayers) {
      qWarning() << "Save game contains invalid data:"
                 << "m_nNumOfPlayers = 0 or CpuScript / Won / StonesLeft "
                    "size != m_nNumOfPlayers.";
      QMessageBox::critical(m_pParent, tr("Warning"),
                            tr("Save game contains invalid data."));
      return false;
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
        QMessageBox::warning(m_pParent, qApp->applicationName(),
                             tr("Save game contains invalid data."));
        return false;
      }
    }

    qDebug() << "Loading save game board:" << m_sBoardFile;

    QJsonArray jsBoard = jsonObj[QStringLiteral("Board")].toArray();
    m_pBoard = new Board(m_pParent, m_nMaxTowerHeight, m_nNumOfPlayers, m_sIN,
                         m_sOUT, m_pSettings);
    if (!m_pBoard->createBoard(m_sBoardFile)) {
      return false;
    }
    if (!m_pBoard->setupSavegame(jsBoard)) {
      qWarning() << "Save game contains invalid data!";
      QMessageBox::warning(m_pParent, qApp->applicationName(),
                           tr("Save game contains invalid data."));
      return false;
    }
  }

  // No save game: Start empty board with default values
  if (nullptr == m_pBoard) {
    m_pBoard = new Board(m_pParent, m_nMaxTowerHeight, m_nNumOfPlayers, m_sIN,
                         m_sOUT, m_pSettings);
    if (!m_pBoard->createBoard(m_sBoardFile)) {
      return false;
    }
    for (int i = 1; i <= m_nNumOfPlayers; i++) {
      Won << 0;
      StonesLeft << m_pBoard->getMaxPlayerStones();
      CpuScript << m_pSettings->getPlayerCpuScript(i);
    }
  }

  connect(m_pBoard, &Board::actionPlayer, this, &Game::makeMove);
  connect(this, &Game::changeZoom, m_pBoard, &Board::changeZoom);

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

  m_pPlayers.reserve(m_nNumOfPlayers);
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    m_pPlayers << new Player(m_pParent, i + 1, m_pBoard->getMaxPlayerStones(),
                             CpuScript.at(i).toString(QStringLiteral("ERROR")));
    m_pPlayers.last()->setStonesLeft(StonesLeft.at(i).toInt(999));
    m_pPlayers.last()->setWonTowers(Won.at(i).toInt(666));

    if (!m_pPlayers.last()->isHuman()) {
      connect(m_pPlayers.last(), &Player::actionCPU, this, &Game::makeMove);
      connect(m_pPlayers.last(), &Player::scriptError, this,
              &Game::caughtScriptError);
    }
  }

  activePlayer.ID = nStartPlayer;
  activePlayer.isHuman = m_pPlayers.at(activePlayer.ID - 1)->isHuman();

  m_previousMove.clear();
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::initCpu() -> bool {
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    if (!m_pPlayers.at(i)->isHuman()) {
      if (!m_pPlayers.at(i)->initCPU(m_pBoard->getBoard(),
                                     m_pBoard->getBoardDimensions(),
                                     m_nMaxTowerHeight, m_nNumOfPlayers,
                                     m_pBoard->getOut(), m_pBoard->getPad())) {
        return false;
      }
    }
  }

  return true;
}

void Game::caughtScriptError() { m_bScriptError = true; }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::getScene() const -> QGraphicsScene * { return m_pBoard; }

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
    bool bValidMove = this->checkMoveIsValid(
        m_pPlayers.at(activePlayer.ID - 1)->getLegalMoves(), move);
    if (bDebug) {
      bValidMove = true;
    }

    if (!bValidMove) {
      if (!activePlayer.isHuman) {
        m_bScriptError = true;
        qWarning() << "Invalid move!"
                   << "P" + QString::number(activePlayer.ID) + " >> [" +
                          QString::number(move.at(0).toInt()) + ", " +
                          QString::number(move.at(1).toInt()) + ", " +
                          QString::number(move.at(2).toInt()) + "]";
        QMessageBox::warning(m_pParent, tr("Warning"),
                             tr("CPU script made an invalid move! "
                                "Please check the debug log."));
      } else {
        if (bSetStone) {
          QMessageBox::information(m_pParent, tr("Information"),
                                   tr("No stones left! Please move a tower."));
        } else {
          QString sMove(m_pBoard->getStringCoordFromIndex(move.at(0).toInt()) +
                        ":" + QString::number(move.at(1).toInt()) + "-" +
                        m_pBoard->getStringCoordFromIndex(move.at(2).toInt()));
          qWarning() << "Invalid move!"
                     << "P" + QString::number(activePlayer.ID) + " >> " + sMove;
          QMessageBox::information(m_pParent, tr("Warning"),
                                   tr("Invalid move!"));
        }
      }
      return;
    }

    if (bSetStone) {
      this->setStone(move.at(2).toInt(), bDebug);
    } else {
      this->moveTower(move.at(0).toInt(), move.at(1).toInt(),
                      move.at(2).toInt());
    }
  } else {
    qWarning() << "Invalid move!" << move;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
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
  // Below checks never should fail, due to comparison with legal moves list...
  if (m_pBoard->getField(nIndex).isEmpty() || bDebug) {
    if (m_pPlayers.at(activePlayer.ID - 1)->getStonesLeft() > 0) {
      m_pPlayers[activePlayer.ID - 1]->setStonesLeft(
          m_pPlayers.at(activePlayer.ID - 1)->getStonesLeft() - 1);
      m_pBoard->addStone(nIndex, activePlayer.ID);
    } else {
      qWarning() << "Invalid move! Set stone:" << nIndex;
      QMessageBox::warning(m_pParent, tr("Warning"),
                           tr("Something went wrong!"));
      return;
    }

    qDebug() << "P" + QString::number(activePlayer.ID) + " >> " +
                    m_pBoard->getStringCoordFromIndex(nIndex);
    m_previousMove.clear();
    this->checkTowerWin(nIndex);
    this->updatePlayers();
  } else {
    qWarning() << "Invalid move! Set stone:" << nIndex;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::moveTower(const int nFrom, const quint8 nStones, const int nTo) {
  QList<int> listStones;
  const QString s(m_pBoard->getField(nFrom));
  listStones.reserve(s.size());
  for (const auto &ch : s) {
    listStones.append(ch.digitValue());
  }

  // Below checks never should fail, due to comparison with legal moves list...
  if (listStones.contains(-1)) {
    qWarning() << "Tower contains invalid stone!" << listStones;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
    return;
  }
  if (listStones.isEmpty()) {
    qWarning() << "Move tower size == 0! Tower:" << nFrom;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
    return;
  }
  if (nStones > listStones.size()) {
    qWarning() << "Trying to move more stones than available! From:" << nFrom
               << "Stones:" << nStones << "To:" << nTo;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
    return;
  }
  if (!m_pBoard->checkNeighbourhood(nTo).contains(nFrom)) {
    qWarning() << "CPU tried to move a tower, which is not in the "
                  "neighbourhood of the selected tower.";
    m_bScriptError = true;
    QMessageBox::warning(m_pParent, tr("Warning"),
                         tr("CPU script made an invalid move! "
                            "Please check the debug log."));
    return;
  }

  // Debug print: E.g. "C4:3-D3" = move 3 stones from C4 to D3 (ASCII 65 = A)
  QString sMove(m_pBoard->getStringCoordFromIndex(nFrom) + ":" +
                QString::number(nStones) + "-" +
                m_pBoard->getStringCoordFromIndex(nTo));
  qDebug() << "P" + QString::number(activePlayer.ID) + " >> " + sMove;
  if (!activePlayer.isHuman) {
    m_pBoard->selectIndexField(nTo);
    m_pBoard->selectIndexField(-1);
  }

  for (int i = 0; i < nStones; i++) {
    m_pBoard->removeStone(nFrom);  // Remove is in the wrong order, nevermind!
    m_pBoard->addStone(nTo, listStones[listStones.size() - nStones + i]);
  }

  m_previousMove.clear();
  m_previousMove << nFrom << nStones << nTo;

  this->checkTowerWin(nTo);
  this->updatePlayers();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::checkTowerWin(const int nIndex) {
  if (m_pBoard->getField(nIndex).size() >= m_nMaxTowerHeight) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    int nWinner(
        QString(QStringView{m_pBoard->getField(nIndex)}.last()).toInt() - 1);
#else
    int nWinner(m_pBoard->getField(nIndex).rightRef(1).toInt() - 1);
#endif
    if (nWinner < 0 || nWinner > m_nNumOfPlayers - 1) {
      qDebug() << Q_FUNC_INFO;
      qWarning() << "Last stone <= 0 or > num of players!";
      qWarning() << "Field:" << nIndex << " -  Tower"
                 << m_pBoard->getField(nIndex);
      QMessageBox::warning(m_pParent, tr("Warning"),
                           tr("Something went wrong!"));
      return;
    }

    m_pPlayers[nWinner]->setWonTowers(m_pPlayers.at(nWinner)->getWonTowers() +
                                      1);
    qDebug() << "Player " + m_pPlayers.at(nWinner)->getID() +
                    " conquered tower " +
                    m_pBoard->getStringCoordFromIndex(nIndex);
    if (m_nTowersToWin != m_pPlayers.at(nWinner)->getWonTowers()) {
      QMessageBox::information(
          m_pParent, tr("Information"),
          tr("%1 conquered a tower!").arg(m_pPlayers.at(nWinner)->getName()));
    }

    this->returnStones(nIndex);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::returnStones(const int nIndex) {
  QString tower(m_pBoard->getField(nIndex));
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    quint8 stones(static_cast<quint8>(tower.count(QString::number(i + 1))));
    m_pPlayers[i]->setStonesLeft(m_pPlayers.at(i)->getStonesLeft() + stones);
  }

  // Clear field
  m_pBoard->removeStone(nIndex, true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::updatePlayers(bool bInitial, bool bDirectionChangesOnce) {
  if (m_bScriptError) {
    emit setInteractive(false);
    return;
  }

  m_TowersNeededToWin = QJsonArray();  // Clear array
  m_NumberOfStones = QJsonArray();
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    emit updateStones(i, QString::number(m_pPlayers.at(i)->getStonesLeft()));
    emit updateWon(i, QString::number(m_pPlayers.at(i)->getWonTowers()) +
                          " / " + QString::number(m_nTowersToWin));
    m_TowersNeededToWin << m_nTowersToWin - m_pPlayers.at(i)->getWonTowers();
    m_NumberOfStones << m_pPlayers.at(i)->getStonesLeft();
  }

  bool bWon(false);
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    if (m_nTowersToWin == m_pPlayers.at(i)->getWonTowers()) {
      bWon = true;
      qDebug() << "PLAYER " + m_pPlayers.at(i)->getID() + " WON!";
      emit setInteractive(false);
      emit highlightActivePlayer(i + 1, i + 1);
      QMessageBox::information(
          m_pParent, tr("Information"),
          tr("%1 won the game!").arg(m_pPlayers.at(i)->getName()));
    }
  }

  if (!bWon) {
    if (bInitial) {
      QStringList sListPlayers;
      for (int i = 0; i < m_nNumOfPlayers; i++) {
        emit drawIcon(i);
        sListPlayers << m_pPlayers.at(i)->getName();
      }
      emit updateNames(sListPlayers);
    } else {  // Toggle active player
      if (bDirectionChangesOnce) {
        activePlayer.ID -= 1;
      } else {
        activePlayer.ID += 1;
      }

      if (activePlayer.ID > m_nNumOfPlayers) {
        activePlayer.ID = 1;
      }
      if (activePlayer.ID < 1) {
        activePlayer.ID = m_nNumOfPlayers;
      }
      activePlayer.isHuman = m_pPlayers.at(activePlayer.ID - 1)->isHuman();
    }

    emit highlightActivePlayer(activePlayer.ID);
    if (!this->checkPossibleMoves()) {
      m_pBoard->printDebugFields();
      return;
    }

    if (!activePlayer.isHuman) {
      emit setInteractive(false);
      QTimer::singleShot(800, this,
                         [this]() { this->delayCpu(m_previousMove); });
    } else {
      emit setInteractive(true);
    }
  }

  m_pBoard->printDebugFields();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Game::delayCpu(const QList<int> &previousMove) {
  QJsonArray prevMove;
  for (const auto &i : previousMove) {
    prevMove << i;
  }

  m_pPlayers.at(activePlayer.ID - 1)
      ->callCpu(m_pBoard->getBoard(),
                m_pPlayers.at(activePlayer.ID - 1)->getLegalMoves(),
                m_TowersNeededToWin, m_NumberOfStones, prevMove);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Game::checkPossibleMoves() -> bool {
  bool bTie(true);
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    m_pPlayers[i]->setLegalMoves(m_pBoard->getLegalMoves(
        m_pPlayers.at(i)->getID(), m_pPlayers.at(i)->getStonesLeft() > 0,
        m_previousMove));

    // Check if one of the players can move
    if (m_pPlayers.at(i)->canMove()) {
      bTie = false;
    }
  }

  if (bTie) {
    emit setInteractive(false);
    qDebug() << "NO MOVES POSSIBLE ANYMORE!";
    bool bSameScore = true;
    for (int i = 0; i < m_nNumOfPlayers - 1; i++) {
      if (m_pPlayers.at(i)->getWonTowers() !=
          m_pPlayers.at(i + 1)->getWonTowers()) {
        bSameScore = false;
        break;
      }
    }
    if (bSameScore) {
      QMessageBox::information(m_pParent, tr("Information"),
                               tr("No moves possible anymore.\n"
                                  "Game ends in a tie!"));
    } else {
      QString sScore(tr("No moves possible anymore! Game ends with score:"));
      for (int i = 0; i < m_nNumOfPlayers; i++) {
        sScore += "\n - " + tr("Player") + " " + QString::number(i + 1) + ": " +
                  QString::number(m_pPlayers.at(i)->getWonTowers());
      }
      QMessageBox::information(m_pParent, tr("Information"), sScore);
    }
    return false;
  }

  if (!m_pPlayers.at(activePlayer.ID - 1)->canMove()) {
    qDebug() << "PLAYER " + m_pPlayers.at(activePlayer.ID - 1)->getID() +
                    " HAS TO PASS!";
    QString sDirectionChange(QLatin1String(""));
    bool bDirectionChangesOnce(false);
    if (m_nNumOfPlayers > 2) {
      quint8 tmpID(activePlayer.ID);
      tmpID -= 1;  // Give back right to move to previous player
      if (tmpID < 1) {
        tmpID = m_nNumOfPlayers;
      }

      // Check if previous player would be able to move.
      // If not, do not change direction, since it would lead to infinity loop.
      if (m_pPlayers.at(tmpID - 1)->canMove()) {
        sDirectionChange =
            "\n" + tr("Right to move is given back to the previous player!");
        qDebug() << "Right to move is given back to the previous player!";
        bDirectionChangesOnce = true;
      }
    }
    QMessageBox::information(
        m_pParent, tr("Information"),
        tr("No move possible! %1 has to pass.")
                .arg(m_pPlayers.at(activePlayer.ID - 1)->getName()) +
            sDirectionChange);
    this->updatePlayers(false, bDirectionChangesOnce);
    return false;
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

  QJsonObject jsonObj;
  jsonObj[QStringLiteral("Board")] = jsBoard;
  jsonObj[QStringLiteral("BoardFile")] = m_sBoardFile;
  jsonObj[QStringLiteral("BoardFileRelative")] = sRelativeDir;
  jsonObj[QStringLiteral("WinTowers")] = m_nTowersToWin;
  jsonObj[QStringLiteral("Current")] = activePlayer.ID;
  jsonObj[QStringLiteral("NumOfPlayers")] = m_nNumOfPlayers;
  QJsonArray arrWon;
  QJsonArray arrStonesLeft;
  QJsonArray arrCPUScript;
  for (int i = 0; i < m_nNumOfPlayers; i++) {
    arrWon << m_pPlayers.at(i)->getWonTowers();
    arrStonesLeft << m_pPlayers.at(i)->getStonesLeft();
    arrCPUScript << m_pPlayers.at(i)->getCpuScript();
  }
  jsonObj[QStringLiteral("Won")] = arrWon;
  jsonObj[QStringLiteral("StonesLeft")] = arrStonesLeft;
  jsonObj[QStringLiteral("CpuScript")] = arrCPUScript;
  QJsonDocument jsDoc(jsonObj);

  return (-1 != saveFile.write(jsDoc.toJson().toBase64()));
}
