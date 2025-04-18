/**
 * \file board.cpp
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
 * Game board generation.
 */

#include "./board.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QTimer>

#include "./settings.h"

Board::Board(QWidget *pParent, const quint8 nMaxTower, quint8 NumOfPlayers,
             const QString &sIN, const QString &sOUT, Settings *pSettings,
             QObject *pParentObj)
    : m_pParent(pParent),
      m_sIN(sIN),
      m_sOUT(sOUT),
      m_sPAD(QStringLiteral("-")),
      m_pSettings(pSettings),
      m_nGridSize(m_pSettings->getGridSize()),
      m_nScale(m_pSettings->getGridSize() / m_pSettings->getDefaultGrid()),
      m_nMaxPlayerStones(0),
      m_nMaxTower(nMaxTower),
      m_NumOfPlayers(NumOfPlayers) {
  Q_UNUSED(pParentObj)
  this->setBackgroundBrush(QBrush(m_pSettings->getBgColor()));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::createBoard(const QString &sBoard) -> bool {
  QList<QString> tmpBoard;
  if (!this->loadBoard(sBoard, tmpBoard)) {
    return false;
  }

  this->addBoardPadding(tmpBoard, m_nMaxTower);
  this->drawBoard(tmpBoard);
  this->createHighlighters();

  if (!this->createStones()) {
    return false;
  }

  /*
   * Moving directions factor
   * E.g. 5x5 board, max tower height 5 (padding):
   * -16 -15 -14
   * -1   X    1
   * 14  15   16
   */
  m_DIRS << -(2 * m_nMaxTower + m_BoardDimensions.x() + 1);  // -16
  m_DIRS << -(2 * m_nMaxTower + m_BoardDimensions.x());      // -15
  m_DIRS << -(2 * m_nMaxTower + m_BoardDimensions.x() - 1);  // -14
  m_DIRS << -1 << 1;                                         // -1, 1
  m_DIRS << -(m_DIRS.at(2));                                 // 14
  m_DIRS << -(m_DIRS.at(1));                                 // 15
  m_DIRS << -(m_DIRS.at(0));                                 // 16

  m_FieldStones.clear();
  m_FieldStones.reserve(m_jsBoard.size());
  QList<QGraphicsSvgItem *> tmpTower;
  for (int i = 0; i < m_jsBoard.size(); i++) {
    m_FieldStones.append(tmpTower);
  }

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::loadBoard(const QString &sBoard, QList<QString> &tmpBoard) -> bool {
  QFile fBoard(sBoard);
  if (!fBoard.exists()) {
    qWarning() << "Board cannot be loaded:" << sBoard;
    QMessageBox::critical(m_pParent, tr("Warning"),
                          tr("Error while opening board file!"));
    return false;
  }

  if (!fBoard.open(QIODevice::ReadOnly)) {
    qWarning() << "Couldn't open open board file:" << sBoard;
    QMessageBox::critical(m_pParent, tr("Warning"),
                          tr("Error while opening board file!"));
    return false;
  }

  QByteArray boardData = fBoard.readAll();
  QJsonDocument loadDoc(QJsonDocument::fromJson(boardData));
  QJsonObject jso = loadDoc.object();
  if (loadDoc.isEmpty() || jso.value(QStringLiteral("Board")).isUndefined() ||
      !jso.value(QStringLiteral("Board")).isArray() ||
      jso.value(QStringLiteral("Columns")).isUndefined() ||
      !jso.value(QStringLiteral("Columns")).isDouble() ||
      jso.value(QStringLiteral("Rows")).isUndefined() ||
      !jso.value(QStringLiteral("Rows")).isDouble() ||
      jso.value(QStringLiteral("PlayersStones")).isUndefined() ||
      !jso.value(QStringLiteral("PlayersStones")).isArray()) {
    qWarning() << "Board file doesn't contain all required keys:" << sBoard;
    qWarning() << "Found json keys:" << jso.keys();
    QMessageBox::critical(m_pParent, tr("Warning"),
                          tr("Error while opening board file!"));
    return false;
  }

  m_BoardDimensions.setX(jso.value(QStringLiteral("Columns")).toInt());
  m_BoardDimensions.setY(jso.value(QStringLiteral("Rows")).toInt());
  if (0 == m_BoardDimensions.x() || 0 == m_BoardDimensions.y()) {
    qWarning() << "Board file contains invalid dimensions:"
               << m_BoardDimensions;
    qWarning() << "Board:" << sBoard;
    QMessageBox::critical(m_pParent, tr("Warning"),
                          tr("Error while opening board file!"));
    return false;
  }

  const QJsonArray stones(jso.value(QStringLiteral("PlayersStones")).toArray());
  if (stones.size() < m_NumOfPlayers) {
    // Use last value as fallback, if number of stones
    // not specified for number of players
    m_nMaxPlayerStones = stones.last().toInt();
    qWarning() << "Player stones not specified for" << m_NumOfPlayers
               << "players. Using fallback:" << m_nMaxPlayerStones;
  } else {
    m_nMaxPlayerStones = stones.at(m_NumOfPlayers - 2).toInt();
  }
  if (0 == m_nMaxPlayerStones) {
    qWarning() << "Number of player stone is 0!";
    QMessageBox::critical(m_pParent, tr("Warning"),
                          tr("Error while opening board file!"));
    return false;
  }

  tmpBoard.clear();
  const QJsonArray jsBoard(jso.value(QStringLiteral("Board")).toArray());
  for (const auto &js : jsBoard) {
    QString s = js.toString();
    if (js.isNull() || s.isEmpty() ||
        (m_sOUT != s && m_sIN != s && m_sPAD != s)) {
      qWarning() << "Board array contains invalid data:" << s;
      qWarning() << "Board:" << sBoard;
      QMessageBox::critical(m_pParent, tr("Warning"),
                            tr("Error while opening board file!"));
      return false;
    }
    tmpBoard << s;
  }

  qDebug() << "Board file:" << sBoard;
  qDebug() << "Board dimensions:" << m_BoardDimensions.x() << "columns x"
           << m_BoardDimensions.y() << "rows";

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::addBoardPadding(const QList<QString> &tmpBoard,
                            const quint8 nMaxTower) {
  // Generate field array
  // Top padding
  for (int i = 0; i < nMaxTower; i++) {
    for (int j = 0; j < (nMaxTower * 2 + m_BoardDimensions.x()); j++) {
      m_jsBoard << m_sPAD;
    }
  }

  // Padding left and right per line
  for (int i = 0; i < tmpBoard.size(); i++) {
    if (0 == i) {  // First item of first line, padding left
      for (int j = 0; j < nMaxTower; j++) {
        m_jsBoard << m_sPAD;
      }
    } else if (0 == i % m_BoardDimensions.x()) {  // First item in row:
      for (int j = 0; j < (nMaxTower * 2);
           j++) {             // Add padding end of previous
        m_jsBoard << m_sPAD;  // & beginning of current line
      }
    }

    if (m_sIN == tmpBoard[i]) {
      m_jsBoard << QString();
    } else {
      m_jsBoard << m_sOUT;
    }

    if (tmpBoard.size() - 1 == i) {  // Last item of last line, padding right
      for (int j = 0; j < nMaxTower; j++) {
        m_jsBoard << m_sPAD;
      }
    }
  }

  // Bottom padding
  for (int i = 0; i < nMaxTower; i++) {
    for (int j = 0; j < (nMaxTower * 2 + m_BoardDimensions.x()); j++) {
      m_jsBoard << m_sPAD;
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::drawBoard(const QList<QString> &tmpBoard) {
  int x(0);
  int y(-m_nGridSize);
  quint8 col(65);
  quint8 row(0);

  for (int i = 0; i < tmpBoard.size(); i++) {
    // m_BoardDimensions.x() = columns, m_BoardDimensions.y() = rows
    if (0 == i % m_BoardDimensions.x()) {  // First item in row
      x = 0;
      col = 65;
      y += m_nGridSize;
      row++;
    } else {
      x += m_nGridSize;
      col++;
    }
    if (m_sOUT == tmpBoard[i]) {
      m_listFields << new QGraphicsRectItem();
      // Rect is not visible anyway, but isVisible property is used during zoom
      m_listFields.last()->setVisible(false);
      continue;
    }

    m_listFields << new QGraphicsRectItem(x, y, m_nGridSize, m_nGridSize);
    m_listFields.last()->setBrush(QBrush(m_pSettings->getBgBoardColor()));
    m_listFields.last()->setPen(QPen(m_pSettings->getGridBoardColor()));
    m_boardPath.addRect(m_listFields.last()->rect());
    this->addItem(m_listFields.last());
    // Field captions for debugging
    if (qApp->arguments().contains(QStringLiteral("--debug"))) {
      m_Captions << this->addSimpleText(QString(static_cast<char>(col)) +
                                        QString::number(row));
      m_Captions.last()->setPos(x + 2, y + m_nGridSize - m_nGridSize / 3);
      m_Captions.last()->setFont(
          QFont(QStringLiteral("Arial"), m_nGridSize / 5));
    }
  }

  // TODO(x): Might need adjustments for non rectangular shapes!
  this->setSceneRect(m_boardPath.boundingRect());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::createHighlighters() {
  // Field highlighter
  m_pHighlightRect = new QGraphicsRectItem(0, 0, m_nGridSize, m_nGridSize);
  m_pHighlightRect->setBrush(QBrush(m_pSettings->getHighlightColor()));
  m_pHighlightRect->setPen(QPen(m_pSettings->getHighlightBorderColor()));
  m_pHighlightRect->setVisible(false);
  m_pHighlightRect->setZValue(0);
  this->addItem(m_pHighlightRect);
  // Selected field
  m_pSelectedField = new QGraphicsRectItem(0, 0, m_nGridSize, m_nGridSize);
  m_pSelectedField->setBrush(QBrush(m_pSettings->getSelectedColor()));
  m_pSelectedField->setPen(QPen(m_pSettings->getSelectedBorderColor()));
  m_pSelectedField->setVisible(false);
  m_pSelectedField->setZValue(0);
  this->addItem(m_pSelectedField);
  // Animation
  m_pAnimateField = new QGraphicsRectItem(0, 0, m_nGridSize, m_nGridSize);
  m_pAnimateField->setBrush(QBrush(m_pSettings->getAnimateColor()));
  m_pAnimateField->setPen(QPen(m_pSettings->getAnimateBorderColor()));
  m_pAnimateField->setVisible(false);
  m_pAnimateField->setZValue(0);
  this->addItem(m_pAnimateField);
  // Animation2
  m_pAnimateField2 = new QGraphicsRectItem(0, 0, m_nGridSize, m_nGridSize);
  m_pAnimateField2->setBrush(QBrush(m_pSettings->getAnimateColor()));
  m_pAnimateField2->setPen(QPen(m_pSettings->getAnimateBorderColor()));
  m_pAnimateField2->setVisible(false);
  m_pAnimateField2->setZValue(0);
  this->addItem(m_pAnimateField2);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::createStones() -> bool {
  // Load svg as txt for below color exchange.
  QFile fStone(QStringLiteral(":/img/stone.svg"));
  if (!fStone.open(QFile::ReadOnly | QFile::Text)) {
    qDebug() << "Could not open stone.svg";
    QMessageBox::critical(
        m_pParent, tr("Warning"),
        tr("Could not open %1!").arg(QStringLiteral("stone.svg")));
    return false;
  }
  QTextStream in(&fStone);
  QString sSvg = in.readAll();
  fStone.close();

  QList<QGraphicsSvgItem *> tmpSvgList;
  // Create a few more than maximum of stones because of wrong
  // order during move tower add/remove
  for (int nPlayers = 0; nPlayers < m_NumOfPlayers; nPlayers++) {
    // stone.svg HAS to be filled with #ff0000, so that below replace can work.
    QString sTmpSvg = sSvg;
    QByteArray aSvg(sTmpSvg
                        .replace(QLatin1String("#ff0000"),
                                 m_pSettings->getPlayerColor(nPlayers))
                        .toUtf8());
    auto *pSvgRenderer = new QSvgRenderer(aSvg);
    tmpSvgList.clear();

    for (int i = 0; i < m_nMaxPlayerStones + 4; i++) {
      tmpSvgList.append(new QGraphicsSvgItem());
      tmpSvgList.last()->setScale(m_nScale);
      tmpSvgList.last()->setSharedRenderer(pSvgRenderer);
      this->addItem(tmpSvgList.last());
      tmpSvgList.last()->setPos(0, 0);
      tmpSvgList.last()->setZValue(5);
      tmpSvgList.last()->setVisible(false);
    }
    m_listPlayerStones << tmpSvgList;
  }

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::changeZoom() {
  m_nGridSize = m_pSettings->getGridSize();
  m_nScale = m_pSettings->getGridSize() / m_pSettings->getDefaultGrid();
  qDebug() << "Zoom - new grid size:" << m_nGridSize;

  int x(0);
  int y(-m_nGridSize);
  quint8 col(65);
  quint8 row(0);

  for (int c = 0; c < m_Captions.size(); c++) {
    this->removeItem(m_Captions[c]);
    delete m_Captions[c];
    m_Captions[c] = nullptr;
  }
  m_Captions.clear();

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  m_boardPath.clear();
#else
  m_boardPath = QPainterPath();
#endif

  // Reposition and rescale board fields
  for (int i = 0; i < m_listFields.size(); i++) {
    // m_BoardDimensions.x() = columns, m_BoardDimensions.y() = rows
    if (0 == i % m_BoardDimensions.x()) {  // First item in row
      x = 0;
      col = 65;
      y += m_nGridSize;
      row++;
    } else {
      x += m_nGridSize;
      col++;
    }
    if (false == m_listFields.at(i)->isVisible()) {  // Is outside
      continue;
    }

    m_listFields[i]->setRect(x, y, m_nGridSize, m_nGridSize);
    m_boardPath.addRect(m_listFields[i]->rect());
    // Field captions for debugging
    if (qApp->arguments().contains(QStringLiteral("--debug"))) {
      m_Captions << this->addSimpleText(QString(static_cast<char>(col)) +
                                        QString::number(row));
      m_Captions.last()->setPos(x + 2, y + m_nGridSize - m_nGridSize / 3);
      m_Captions.last()->setFont(
          QFont(QStringLiteral("Arial"), m_nGridSize / 5));
    }
  }

  // Rescale highlighters
  m_pHighlightRect->setRect(0, 0, m_nGridSize, m_nGridSize);
  m_pSelectedField->setRect(0, 0, m_nGridSize, m_nGridSize);
  m_pAnimateField->setRect(0, 0, m_nGridSize, m_nGridSize);
  m_pAnimateField2->setRect(0, 0, m_nGridSize, m_nGridSize);

  // Not yet placed player stones
  for (int nPlayers = 0; nPlayers < m_NumOfPlayers; nPlayers++) {
    for (int k = 0; k < m_listPlayerStones[nPlayers].size(); k++) {
      m_listPlayerStones[nPlayers][k]->setScale(m_nScale);
    }
  }

  // Stones on board
  for (int nIndex = 0; nIndex < m_FieldStones.size(); nIndex++) {
    for (int nNum = 0; nNum < m_FieldStones.at(nIndex).size(); nNum++) {
      m_FieldStones[nIndex][nNum]->setScale(m_nScale);

      // Adjust position
      QRectF rect(m_FieldStones[nIndex][nNum]->boundingRect());
      m_FieldStones[nIndex][nNum]->setPos(this->getCoordinateFromIndex(nIndex) *
                                          m_nGridSize);
      m_FieldStones[nIndex][nNum]->setPos(
          m_FieldStones[nIndex][nNum]->x() -
              (rect.width() / 4 * m_nScale - 3 * m_nScale) * nNum,
          m_FieldStones[nIndex][nNum]->y() -
              (rect.height() / 4 * m_nScale - 3 * m_nScale) * nNum);
    }
  }

  // Redraw board
  this->update(QRectF(0, 0, m_BoardDimensions.x() * m_nGridSize - 1,
                      m_BoardDimensions.y() * m_nGridSize - 1));

  // TODO(x): Might need adjustments for non rectangular shapes!
  this->setSceneRect(m_boardPath.boundingRect());
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::setupSavegame(const QJsonArray &jsBoard) -> bool {
  if (jsBoard.size() != m_jsBoard.size()) {
    qWarning() << "jsBoard.size() != m_jsBoard.size()";
    return false;
  }

  for (int i = 0; i < jsBoard.size(); i++) {
    const QString s = jsBoard.at(i).toString();
    if (!s.isEmpty() && m_sPAD != s && m_sOUT != s) {
      for (const auto &ch : s) {
        if (-1 != ch.digitValue() && m_sPAD != m_jsBoard.at(i).toString() &&
            m_sOUT != m_jsBoard.at(i).toString()) {
          this->addStone(i, ch.digitValue());
        } else {
          qWarning() << "Save game data invalid stone at index" << i;
          qWarning() << "Character:" << ch;
          return false;
        }
      }
    } else if (s.isEmpty()) {
      m_jsBoard[i] = "";
    } else {  // Should be Padding or Out
      if (jsBoard.at(i) != m_jsBoard.at(i)) {
        qWarning() << "Save game data != board array at index" << i;
        qWarning() << "Save game:" << jsBoard.at(i)
                   << "- board:" << m_jsBoard.at(i);
        return false;
      }
    }
  }

  // Redraw board
  this->update(QRectF(0, 0, m_BoardDimensions.x() * m_nGridSize - 1,
                      m_BoardDimensions.y() * m_nGridSize - 1));
  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
  if (m_boardPath.contains(p_Event->scenePos())) {
    const QList<QGraphicsItem *> items = this->items(p_Event->scenePos());
    QJsonArray move;
    for (const auto &item : items) {
      int field =
          m_listFields.indexOf(qgraphicsitem_cast<QGraphicsRectItem *>(item));
      if (field > -1) {
        /*
        qDebug() << "Clicked field:" << field;
        qDebug() << "Board index:  " << getIndexFromField(field);
        qDebug() << "Coordinate:   " << getCoordinateFromField(field);
        qDebug() << "String coord.:" << getStringCoordFromField(field);
        */

        // If debug enabled, use Ctrl + right mouse button to set stone anywhere
        if (Qt::RightButton == p_Event->button() &&
            Qt::ControlModifier == p_Event->modifiers() &&
            qApp->arguments().contains(QStringLiteral("--debug"))) {
          this->selectIndexField(-1);
          qDebug() << "Following stone set in DEBUG mode:";
          move << -999 << 1 << getIndexFromField(field);
          emit actionPlayer(move);
          break;
        }

        // Place tower, if field is empty
        if (m_jsBoard.at(getIndexFromField(field)).toString().isEmpty()) {
          this->selectIndexField(-1);
          move << -1 << 1 << getIndexFromField(field);
          emit actionPlayer(move);
        } else {  // Otherwise select / move tower
          this->selectIndexField(getIndexFromField(field));
        }
        break;
      }
    }
  }

  QGraphicsScene::mousePressEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event) {
  static QPointF point;

  if (m_boardPath.contains(p_Event->scenePos())) {
    m_pHighlightRect->setVisible(true);
    point = p_Event->scenePos();
    point = QPointF(point.x() - m_nGridSize / 2, point.y() - m_nGridSize / 2);
    m_pHighlightRect->setPos(this->snapToGrid(point));
  } else {
    m_pHighlightRect->setVisible(false);
  }

  QGraphicsScene::mouseMoveEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::snapToGrid(const QPointF point) const -> QPointF {
  return QPointF(qRound(point.x() / m_nGridSize) * m_nGridSize,
                 qRound(point.y() / m_nGridSize) * m_nGridSize);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getIndexFromField(const int nField) const -> int {
  const int nTop = m_nMaxTower * (2 * m_nMaxTower + m_BoardDimensions.x());
  const int nFirst = nTop + m_nMaxTower;
  int nLeftRight = 2 * m_nMaxTower * (nField / m_BoardDimensions.x());
  int nCol = nField % m_BoardDimensions.x();
  int nRow = (nField / m_BoardDimensions.x()) * m_BoardDimensions.x();
  return nFirst + nLeftRight + nCol + nRow;
}

auto Board::getFieldFromIndex(const int nIndex) const -> int {
  const int nTop = m_nMaxTower * (2 * m_nMaxTower + m_BoardDimensions.x());
  const int nFirst = nTop + m_nMaxTower;
  int nLeftRight =
      2 * m_nMaxTower *
      ((nIndex / (m_BoardDimensions.x() + 2 * m_nMaxTower)) - m_nMaxTower);
  return nIndex - nFirst - nLeftRight;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getCoordinateFromField(const int nField) const -> QPoint {
  int x = nField % m_BoardDimensions.x();
  int y = nField / m_BoardDimensions.x();
  return QPoint(x, y);
}

auto Board::getStringCoordFromField(const int nField) const -> QString {
  QPoint point(this->getCoordinateFromField(nField));
  return QString(static_cast<char>(point.x() + 65) +
                 QString::number(point.y() + 1));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getCoordinateFromIndex(const int nIndex) const -> QPoint {
  return this->getCoordinateFromField(this->getFieldFromIndex(nIndex));
}

auto Board::getStringCoordFromIndex(const int nIndex) const -> QString {
  return this->getStringCoordFromField(this->getFieldFromIndex(nIndex));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::addStone(const int nIndex, const quint8 nStone, const bool bAnim) {
  auto nExisting(static_cast<quint8>(m_jsBoard.at(nIndex).toString().size()));

  if (nStone < 1 || nStone > m_NumOfPlayers) {
    qWarning() << "Trying to set invalid stone type" << nStone;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
    return;
  }

  m_jsBoard[nIndex] = m_jsBoard.at(nIndex).toString() + QString::number(nStone);

  m_FieldStones[nIndex].append(m_listPlayerStones.at(nStone - 1).last());
  m_listPlayerStones[nStone - 1].removeLast();

  if (bAnim) {
    this->startAnimation(this->getCoordinateFromIndex(nIndex));
  }

  QRectF rect(m_FieldStones[nIndex].last()->boundingRect());
  m_FieldStones[nIndex].last()->setPos(this->getCoordinateFromIndex(nIndex) *
                                       m_nGridSize);
  m_FieldStones[nIndex].last()->setPos(
      m_FieldStones[nIndex].last()->x() -
          (rect.width() / 4 * m_nScale - 3 * m_nScale) * nExisting,
      m_FieldStones[nIndex].last()->y() -
          (rect.height() / 4 * m_nScale - 3 * m_nScale) * nExisting);

  m_FieldStones[nIndex].last()->setVisible(true);

  for (int z = 0; z < m_FieldStones.at(nIndex).size(); z++) {
    m_FieldStones[nIndex][z]->setZValue(6 + z);
  }

  if (bAnim) {
    // Redraw board
    this->update(QRectF(0, 0, m_BoardDimensions.x() * m_nGridSize - 1,
                        m_BoardDimensions.y() * m_nGridSize - 1));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::startAnimation(const QPoint field) {
  m_pAnimateField->setPos(this->snapToGrid(field * m_nGridSize));
  m_pAnimateField->setVisible(true);
  m_pHighlightRect->setVisible(false);
  QTimer::singleShot(500, this, &Board::resetAnimation);
}

void Board::resetAnimation() { m_pAnimateField->setVisible(false); }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::startAnimation2(const QPoint field) {
  m_pAnimateField2->setPos(this->snapToGrid(field * m_nGridSize));
  m_pAnimateField2->setVisible(true);
  m_pHighlightRect->setVisible(false);
  QTimer::singleShot(500, this, &Board::resetAnimation2);
}

void Board::resetAnimation2() { m_pAnimateField2->setVisible(false); }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::removeStone(const int nIndex, const bool bAll) {
  if (m_jsBoard.at(nIndex).toString().isEmpty()) {
    qWarning() << "Trying to remove stone from empty field" << nIndex;
    QMessageBox::warning(m_pParent, tr("Warning"), tr("Something went wrong!"));
    return;
  }

  if (bAll) {  // Remove all (tower conquered)
    const QString s(this->getField(nIndex));
    for (const auto &ch : s) {
      // For starts at the beginning of the list
      m_listPlayerStones[ch.digitValue() - 1].append(
          m_FieldStones[nIndex].first());
      m_FieldStones[nIndex].first()->setVisible(false);
      m_FieldStones[nIndex].removeFirst();
    }
    m_jsBoard[nIndex] = QString();
  } else {  // Remove only one
    int nPlayer(
        QString(QStringView{m_jsBoard.at(nIndex).toString()}.last()).toInt());

    m_listPlayerStones[nPlayer - 1].append(m_FieldStones[nIndex].last());
    m_FieldStones[nIndex].last()->setVisible(false);
    m_FieldStones[nIndex].removeLast();

    QString s(m_jsBoard.at(nIndex).toString());
    s.chop(1);
    m_jsBoard[nIndex] = s;
  }
  // Redraw board
  this->update(QRectF(0, 0, m_BoardDimensions.x() * m_nGridSize - 1,
                      m_BoardDimensions.y() * m_nGridSize - 1));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getField(const int index) const -> QString {
  return m_jsBoard.at(index).toString();
}

auto Board::getBoard() const -> QJsonArray { return m_jsBoard; }

auto Board::getBoardDimensions() const -> QPoint { return m_BoardDimensions; }

auto Board::getMaxPlayerStones() const -> quint8 { return m_nMaxPlayerStones; }

auto Board::getOut() const -> QString { return m_sOUT; }

auto Board::getPad() const -> QString { return m_sPAD; }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::selectIndexField(const int nIndex) {
  static int currentIndex(-1);
  QList<int> neighbours;

  if (-1 == nIndex) {
    currentIndex = -1;
    m_pSelectedField->setVisible(false);
    this->highlightNeighbourhood(neighbours);
    // qDebug() << "Deselected";
    return;
  }

  if (currentIndex == nIndex || m_jsBoard.at(nIndex).toString().isEmpty()) {
    currentIndex = -1;
    m_pSelectedField->setVisible(false);
    this->highlightNeighbourhood(neighbours);
    // qDebug() << "Deselected";
    return;
  }

  QJsonArray move;
  neighbours = this->checkNeighbourhood(currentIndex);
  if (neighbours.contains(nIndex) && m_pSelectedField->isVisible()) {  // Move
    neighbours.clear();
    this->highlightNeighbourhood(neighbours);
    m_pSelectedField->setVisible(false);
    this->startAnimation2(this->getCoordinateFromIndex(nIndex));

    int nStonesToMove = 1;
    if (m_jsBoard.at(nIndex).toString().length() > 1) {
      bool ok;
      nStonesToMove = QInputDialog::getInt(
          m_pParent, tr("Move tower"), tr("How many stones shall be moved:"), 1,
          1, m_jsBoard.at(nIndex).toString().length(), 1, &ok);
      if (!ok) {
        return;
      }
    }

    move << nIndex << nStonesToMove << currentIndex;
    emit actionPlayer(move);
    currentIndex = -1;
  } else {  // Select
    currentIndex = nIndex;
    m_pSelectedField->setVisible(true);
    m_pSelectedField->setRect(
        m_listFields.at(this->getFieldFromIndex(nIndex))->rect());
    if (m_pSettings->getShowPossibleMoveTowers()) {
      this->highlightNeighbourhood(this->checkNeighbourhood(currentIndex));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::checkNeighbourhood(const int nIndex) const -> QList<int> {
  QList<int> neighbours;
  if (-1 == nIndex) {
    return neighbours;
  }

  auto nMoves = static_cast<quint8>(m_jsBoard.at(nIndex).toString().size());
  // qDebug() << "Sel:" << getStringCoordFromIndex(nIndex) << " Mov:" << nMoves;

  QString sField;
  for (int dir = 0; dir < m_DIRS.size(); dir++) {
    for (int range = 1; range <= nMoves; range++) {
      sField = m_jsBoard.at(nIndex + m_DIRS.at(dir) * range).toString();
      if (!sField.isEmpty() && range < nMoves) {
        // qDebug() << "Route blocked";
        break;
      }
      if (0 != sField.toInt() && range == nMoves) {
        neighbours.append(nIndex + m_DIRS.at(dir) * range);
      }
    }
  }

  // qDebug() << "Neighbours which can be moved:" << neighbours;
  return neighbours;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::highlightNeighbourhood(const QList<int> &neighbours) {
  static QList<QGraphicsRectItem *> listPossibleMoves;

  for (auto *rect : listPossibleMoves) {
    delete rect;
  }
  listPossibleMoves.clear();

  listPossibleMoves.reserve(neighbours.size());
  for (auto nIndex : neighbours) {
    QPoint point(this->getCoordinateFromIndex(nIndex));
    listPossibleMoves << new QGraphicsRectItem(point.x() * m_nGridSize,
                                               point.y() * m_nGridSize,
                                               m_nGridSize, m_nGridSize);
    listPossibleMoves.last()->setBrush(
        QBrush(m_pSettings->getNeighboursColor()));
    listPossibleMoves.last()->setPen(
        QPen(m_pSettings->getNeighboursBorderColor()));
    listPossibleMoves.last()->setVisible(true);
    this->addItem(listPossibleMoves.last());
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getLegalMoves(const QString &sID, const bool bStonesLeft,
                          const QList<int> &lastMove) const -> QJsonDocument {
  QVariantList varMove;
  QJsonArray move;
  QJsonArray jsMoves;
  int nField = -1;
  int nTo;
  QString s;

  for (int nRow = 0; nRow < m_BoardDimensions.y(); nRow++) {
    for (int nCol = 0; nCol < m_BoardDimensions.x(); nCol++) {
      nField++;
      nTo = this->getIndexFromField(nField);
      s = m_jsBoard.at(nTo).toString();
      if (m_sOUT != s && m_sPAD != s) {
        if (s.isEmpty() && bStonesLeft) {  // Set stone on empty field
          varMove.clear();
          varMove << -1 << 1 << nTo;
          move = QJsonArray::fromVariantList(varMove);
          jsMoves.append(move);
          continue;
        }

        const QList<int> neighbours = this->checkNeighbourhood(nTo);
        if (!neighbours.isEmpty()) {  // Possible tower moves
          for (const auto nFrom : neighbours) {
            const QString sTower = m_jsBoard.at(nFrom).toString();
            for (int nStones = 1;
                 nStones <= m_jsBoard.at(nFrom).toString().size(); nStones++) {
              // Skip suicide moves (opponent wins) for > 2 players
              if (m_NumOfPlayers > 2) {
                if ((m_jsBoard.at(nTo).toString().size() + nStones >=
                     m_nMaxTower) &&
                    QString(QStringView{sTower}.last()) != sID) {
                  continue;
                }
              }

              varMove.clear();
              varMove << nFrom << nStones << nTo;
              move = QJsonArray::fromVariantList(varMove);

              // Previous move reverted?
              if (lastMove.size() == 3) {
                if (!(nFrom == lastMove[2] &&    // From = previous to
                      nStones == lastMove[1] &&  // Same number of stones moved
                      nTo == lastMove[0])) {     // To = previous from
                  jsMoves.append(move);
                }
              } else {
                jsMoves.append(move);
              }
            }
          }
        }
      }
    }
  }

  QJsonDocument legalMoves(jsMoves);
  if (jsMoves.isEmpty()) {
    return QJsonDocument();
  }
  return legalMoves;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::printDebugFields() const {
  qDebug() << "BOARD:";
  QString sLine;
  int nField = -1;
  QString s;

  for (int nRow = 0; nRow < m_BoardDimensions.y(); nRow++) {
    sLine.clear();
    for (int nCol = 0; nCol < m_BoardDimensions.x(); nCol++) {
      nField++;
      s = "(" + m_jsBoard.at(this->getIndexFromField(nField)).toString() + ")";
      if (QString("(" + m_sOUT + ")") == s) {
        s = m_sOUT + m_sOUT;
      }
      sLine += s;
      if (nCol < m_BoardDimensions.x() - 1) {
        sLine += QLatin1String(" ");
      }
    }
    qDebug() << sLine;
  }
}
