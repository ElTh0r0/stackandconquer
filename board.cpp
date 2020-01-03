/**
 * \file board.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2020 Thorsten Roth <elthoro@gmx.de>
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
 * Game board generation.
 */

#include "./board.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTimer>

Board::Board(QPoint NumOfFields, quint16 nGridSize,
             quint8 nMaxStones, Settings *pSettings)
  : sIN("0"),
    sOUT("#"),
    m_nGridSize(nGridSize),
    m_nMaxStones(nMaxStones),
    m_pSettings(pSettings),
    m_NumOfFields(NumOfFields),
    m_pSvgRenderer(nullptr) {
  this->setBackgroundBrush(QBrush(m_pSettings->getBgColor()));

//  this->loadBoard("./square_2x2.stackboard");
  this->loadBoard("./square_5x5.stackboard");
//  this->loadBoard("./triangle.stackboard");

  this->drawBoard();
  this->createHighlighters();
  this->createStones();

  // Generate field matrix
  /*
   * Attention: For genrating a matrix m_Fields[nCol][nRow] the below logic
   * for generating the board is "column by column" and NOT "row by row":
   * m_Fields[nCol] "contains" nRow x elements
   */

  // TODO(): Implement new board array!

  QList<quint8> tower;
  QList<QList<quint8> > column;
  column.reserve(m_NumOfFields.y());
  QList<QGraphicsSvgItem *> tower2;
  QList<QList<QGraphicsSvgItem *> > column2;
  column2.reserve(m_NumOfFields.y());
  for (int i = 0; i < m_NumOfFields.y(); i++) {  // Column "height"
    column.append(tower);
    column2.append(tower2);
  }
  m_Fields.clear();
  m_Fields.reserve(m_NumOfFields.x());
  m_FieldStones.clear();
  m_FieldStones.reserve(m_NumOfFields.x());
  for (int i = 0; i < m_NumOfFields.x(); i++) {  // Number of columns
    m_Fields.append(column);
    m_FieldStones.append(column2);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

bool Board::loadBoard(const QString &sBoard) {
  QFile fBoard(sBoard);
  if (!fBoard.exists()) {
    qWarning() << "Board cannot be loaded:" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return false;
  }

  if (!fBoard.open(QIODevice::ReadOnly)) {
    qWarning() << "Couldn't open open board file:" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return false;
  }

  QByteArray boardData = fBoard.readAll();
  QJsonDocument loadDoc(QJsonDocument::fromJson(boardData));
  QJsonObject jso = loadDoc.object();
  if (loadDoc.isEmpty() ||
      jso.value("Board").isUndefined() || !jso.value("Board").isArray() ||
      jso.value("Columns").isUndefined() || !jso.value("Columns").isDouble() ||
      jso.value("Rows").isUndefined() || !jso.value("Rows").isDouble()) {
    //TODO(): Extent check for all neeeded sections
    qWarning() << "Board file doesn't contain all required sections!" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return false;
  }

  QString s;
  foreach (QJsonValue js, jso.value("Board").toArray()) {
    s = js.toString().trimmed();
    if (js.isNull() || s.isEmpty() || (sOUT != s && sIN != s)) {
      qWarning() << "Board array contains invalid data:" << s;
      qWarning() << "Board:" << sBoard;
      QMessageBox::critical(nullptr, tr("Warning"),
                           tr("Error while opening board file!"));
      return false;
    }
    m_Board << s;
  }

  m_BoardDimension.setX(jso.value("Columns").toInt());
  m_BoardDimension.setY(jso.value("Rows").toInt());

  if (0 == m_BoardDimension.x() || 0 == m_BoardDimension.y()) {
    qWarning() << "Board file contains invalid dimension:" << m_BoardDimension;
    qWarning() << "Board:" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return false;
  }

  //TODO(): Add all fields from json file

  qDebug() << "Board dimensions:" << m_BoardDimension.x() << "columns x"
           << m_BoardDimension.y() << "rows";

  return true;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::drawBoard() {
  quint16 x(0);
  quint16 y(0);
  quint8 col(65);
  quint8 row(0);

  for (int i = 0; i < m_Board.size(); i++) {
    if (0 == i % m_BoardDimension.x()) {  // Start of new col
      x = 0;
      col = 65;
    } else {
      x += m_nGridSize;
      col++;
    }
    if (0 == i % m_BoardDimension.y()) {  // New row
      y += m_nGridSize;
      row++;
    }
    if (sOUT == m_Board[i]) {
      continue;
    }

    m_listFields << new QGraphicsRectItem(x, y, m_nGridSize, m_nGridSize);
    m_listFields.last()->setBrush(QBrush(m_pSettings->getBgBoardColor()));
    m_listFields.last()->setPen(QPen(m_pSettings->getGridBoardColor()));
    m_listFields.last()->setAcceptHoverEvents(true);
    m_boardPath.addRect(m_listFields.last()->rect());
    this->addItem(m_listFields.last());

    // Field captions for debugging
    if (qApp->arguments().contains(QStringLiteral("--debug"))) {
      m_Captions << this->addSimpleText(
                      QString(static_cast<char>(col)) + QString::number(row));
      m_Captions.last()->setPos(x+2, y + m_nGridSize - m_nGridSize/3);
      m_Captions.last()->setFont(QFont(QStringLiteral("Arial"), m_nGridSize/5));
    }
  }

  // TODO(): Might need adjustments for non rectangular shapes!
  this->setSceneRect(this->itemsBoundingRect());
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

void Board::createStones() {
  QSvgRenderer *m_pSvgRenderer = new QSvgRenderer(
                                   QStringLiteral(":/images/stones.svg"));
  // Create a few more than maximum of stones because of wrong
  // order during move tower add/remove
  for (int i = 0; i < m_nMaxStones + 4; i++) {
    m_listStonesP1.append(new QGraphicsSvgItem());
    // Don't transform graphics to isometric view!
    m_listStonesP1.last()->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    m_listStonesP1.last()->setSharedRenderer(m_pSvgRenderer);
    m_listStonesP1.last()->setElementId(QStringLiteral("Stone1"));
    this->addItem(m_listStonesP1.last());
    m_listStonesP1.last()->setPos(0, 0);
    m_listStonesP1.last()->setZValue(5);
    m_listStonesP1.last()->setVisible(false);

    m_listStonesP2.append(new QGraphicsSvgItem());
    // Don't transform graphics to isometric view!
    m_listStonesP2.last()->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    m_listStonesP2.last()->setSharedRenderer(m_pSvgRenderer);
    m_listStonesP2.last()->setElementId(QStringLiteral("Stone2"));
    this->addItem(m_listStonesP2.last());
    m_listStonesP2.last()->setPos(0, 0);
    m_listStonesP2.last()->setZValue(5);
    m_listStonesP2.last()->setVisible(false);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::setupSavegame(const QList<QList<QList<quint8> > > &board) {
  for (int nRow = 0; nRow < m_NumOfFields.y(); nRow++) {
    for (int nCol = 0; nCol < m_NumOfFields.x(); nCol++) {
      foreach (quint8 stone, board[nCol][nRow]) {
        this->addStone(QPoint(nCol, nRow), stone);
      }
    }
  }

  // Redraw board
  this->update(QRectF(0, 0, m_NumOfFields.x() * m_nGridSize-1,
                      m_NumOfFields.y() * m_nGridSize-1));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
  if (m_boardPath.contains(p_Event->scenePos())) {
    // qDebug() << "Mouse POS:" << p_Event->scenePos();
    // qDebug() << "SNAP:" << this->snapToGrid(p_Event->scenePos());
    // qDebug() << "GRID:" << this->getGridField(p_Event->scenePos());

    // TODO(): Implement new board array!

    // Place tower, if field is empty
    if (this->getField(this->getGridField(p_Event->scenePos())).isEmpty()) {
      this->selectField(QPointF(-1, -1));
      emit setStone(this->getGridField(p_Event->scenePos()));
    } else {  // Otherwise select / move tower
      this->selectField(p_Event->scenePos());
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
    point = QPointF(point.x() - m_nGridSize/2, point.y() - m_nGridSize/2);
    m_pHighlightRect->setPos(this->snapToGrid(point));
  } else {
    m_pHighlightRect->setVisible(false);
  }

  QGraphicsScene::mouseMoveEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPointF Board::snapToGrid(const QPointF point) const {
  return QPointF(qRound(point.x() / m_nGridSize) * m_nGridSize,
                 qRound(point.y() / m_nGridSize) * m_nGridSize);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPoint Board::getGridField(const QPointF point) const {
  qint8 x(static_cast<qint8>(point.toPoint().x() / m_nGridSize));
  qint8 y(static_cast<qint8>(point.toPoint().y() / m_nGridSize));

  if (x < 0 || y < 0 || x >= m_NumOfFields.x() || y >= m_NumOfFields.y()) {
    qWarning() << "Point out of grid! (" << x << "," << y << ")";
    if (x < 0) { x = 0; }
    if (y < 0) { y = 0; }
    if (x >= m_NumOfFields.x()) {
      x = static_cast<qint8>(m_NumOfFields.x() - 1);
    }
    if (y >= m_NumOfFields.y()) {
      y = static_cast<qint8>(m_NumOfFields.y() - 1);
    }
    qWarning() << "Changed point to (" << x << "," << y << ")";
  }

  return QPoint(x, y);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::addStone(const QPoint field, const quint8 stone, const bool bAnim) {
  quint8 nExisting(static_cast<quint8>(m_Fields[field.x()][field.y()].size()));

  if (1 == stone) {
    m_Fields[field.x()][field.y()].append(stone);
    m_FieldStones[field.x()][field.y()].append(m_listStonesP1.last());
    m_listStonesP1.removeLast();
  } else if (2 == stone) {
    m_Fields[field.x()][field.y()].append(stone);
    m_FieldStones[field.x()][field.y()].append(m_listStonesP2.last());
    m_listStonesP2.removeLast();
  } else {
    qWarning() << "Trying to set stone type" << stone;
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
    return;
  }

  if (bAnim) {
    this->startAnimation(field);
  }

  m_FieldStones[field.x()][field.y()].last()->setPos(field*m_nGridSize);
  m_FieldStones[field.x()][field.y()].last()->setPos(
        m_FieldStones[field.x()][field.y()].last()->x() - 16 - 13*nExisting,
      m_FieldStones[field.x()][field.y()].last()->y() + 20 - 13*nExisting);
  m_FieldStones[field.x()][field.y()].last()->setVisible(true);

  for (int z = 0; z < m_FieldStones[field.x()][field.y()].size(); z++) {
    m_FieldStones[field.x()][field.y()][z]->setZValue(6 + z);
  }

  if (bAnim) {
    // Redraw board
    this->update(QRectF(0, 0, m_NumOfFields.x() * m_nGridSize-1,
                        m_NumOfFields.y() * m_nGridSize-1));
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::startAnimation(const QPoint field) {
  m_pAnimateField->setPos(this->snapToGrid(field*m_nGridSize));
  m_pAnimateField->setVisible(true);
  m_pHighlightRect->setVisible(false);
  QTimer::singleShot(500, this, &Board::resetAnimation);
}

void Board::resetAnimation() {
  m_pAnimateField->setVisible(false);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::startAnimation2(const QPoint field) {
  m_pAnimateField2->setPos(this->snapToGrid(field*m_nGridSize));
  m_pAnimateField2->setVisible(true);
  m_pHighlightRect->setVisible(false);
  QTimer::singleShot(500, this, &Board::resetAnimation2);
}

void Board::resetAnimation2() {
  m_pAnimateField2->setVisible(false);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::removeStone(const QPoint field, const bool bAll) {
  if (0 == m_Fields[field.x()][field.y()].size()) {
    qWarning() << "Trying to remove stone from empty field" << field;
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
    return;
  } else if (bAll) {  // Remove all (tower conquered)
    foreach (quint8 i, m_Fields[field.x()][field.y()]) {
      // Foreach starts at the beginning of the list
      if (1 == i) {  // Player 1
        m_listStonesP1.append(m_FieldStones[field.x()][field.y()].first());
        m_FieldStones[field.x()][field.y()].first()->setVisible(false);
        m_FieldStones[field.x()][field.y()].removeFirst();
      } else {  // Player 2
        m_listStonesP2.append(m_FieldStones[field.x()][field.y()].first());
        m_FieldStones[field.x()][field.y()].first()->setVisible(false);
        m_FieldStones[field.x()][field.y()].removeFirst();
      }
    }
    m_Fields[field.x()][field.y()].clear();
  } else {  // Remove only one
    if (1 == m_Fields[field.x()][field.y()].last()) {  // Player 1
      m_listStonesP1.append(m_FieldStones[field.x()][field.y()].last());
      m_FieldStones[field.x()][field.y()].last()->setVisible(false);
      m_FieldStones[field.x()][field.y()].removeLast();
    } else {  // Player 2
      m_listStonesP2.append(m_FieldStones[field.x()][field.y()].last());
      m_FieldStones[field.x()][field.y()].last()->setVisible(false);
      m_FieldStones[field.x()][field.y()].removeLast();
    }
    m_Fields[field.x()][field.y()].removeLast();
  }
  // Redraw board
  this->update(QRectF(0, 0, m_NumOfFields.x() * m_nGridSize-1,
                      m_NumOfFields.y() * m_nGridSize-1));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QList<QList<QList<quint8> > > Board::getBoard() const {
  return m_Fields;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QList<quint8> Board::getField(const QPoint field) const {
  return m_Fields[field.x()][field.y()];
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::selectField(const QPointF point) {
  static QPoint currentField(QPoint(-1, -1));
  QPointF pointSnap(point);
  pointSnap = QPointF(pointSnap.x() - m_nGridSize/2,
                      pointSnap.y() - m_nGridSize/2);
  pointSnap = this->snapToGrid(pointSnap);
  QList<QPoint> neighbours;

  if (QPointF(-1, -1) == point) {
    currentField = QPoint(-1, -1);
    m_pSelectedField->setVisible(false);
    this->highlightNeighbourhood(neighbours);
    // qDebug() << "Deselected";
    return;
  } else {
    QPoint field = this->getGridField(point);
    if (currentField == field ||
        0 == m_Fields[field.x()][field.y()].size()) {
      currentField = QPoint(-1, -1);
      m_pSelectedField->setVisible(false);
      this->highlightNeighbourhood(neighbours);
      // qDebug() << "Deselected";
      return;
    }
    neighbours = this->checkNeighbourhood(currentField);
    if (neighbours.contains(field) && m_pSelectedField->isVisible()) {  // Move
      neighbours.clear();
      this->highlightNeighbourhood(neighbours);
      m_pSelectedField->setVisible(false);
      this->startAnimation2(field);
      emit moveTower(field, currentField, 0);
      currentField = QPoint(-1, -1);
    } else {  // Select
      currentField = field;
      m_pSelectedField->setVisible(true);
      m_pSelectedField->setPos(pointSnap);
      if (m_pSettings->getShowPossibleMoveTowers()) {
        this->highlightNeighbourhood(
              this->checkNeighbourhood(currentField));
      }
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QList<QPoint> Board::checkNeighbourhood(const QPoint field) const {
  QList<QPoint> neighbours;
  if (QPoint(-1, -1) == field) {
    return neighbours;
  }

  quint8 nMoves = static_cast<quint8>(m_Fields[field.x()][field.y()].size());
  // qDebug() << "Selected:" << field << "- Moves:" << nMoves;

  for (int y = field.y() - nMoves; y <= field.y() + nMoves; y += nMoves) {
    for (int x = field.x() - nMoves; x <= field.x() + nMoves; x += nMoves) {
      if (x < 0 || y < 0 || x >= m_NumOfFields.x() || y >= m_NumOfFields.y() ||
          field == QPoint(x, y)) {
        continue;
      } else if (m_Fields[x][y].size() > 0) {
        // Check for blocking towers in between
        QPoint check(x, y);
        // qDebug() << "POSSIBLE:" << check;
        QPoint route(field - check);
        bool bBreak = false;

        for (int i = 1; i < nMoves; i++) {
          if (route.y() < 0) {
            check.setY(check.y() - 1);
          } else if (route.y() > 0) {
            check.setY(check.y() + 1);
          } else {
            check.setY(y);
          }

          if (route.x() < 0) {
            check.setX(check.x() - 1);
          } else if (route.x() > 0) {
            check.setX(check.x() + 1);
          } else {
            check.setX(x);
          }

          // qDebug() << "Check route:" << check;
          if (m_Fields[check.x()][check.y()].size() > 0) {
            // qDebug() << "Route blocked";
            bBreak = true;
            break;
          }
        }

        if (false == bBreak) {
          neighbours.append(QPoint(x, y));
        }
      }
    }
  }
  // qDebug() << "Neighbours which can be moved:" << neighbours;
  return neighbours;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::highlightNeighbourhood(const QList<QPoint> &neighbours) {
  static QList<QGraphicsRectItem *> listPossibleMoves;

  foreach (QGraphicsRectItem *rect, listPossibleMoves) {
    delete rect;
  }
  listPossibleMoves.clear();

  foreach (QPoint posField, neighbours) {
    listPossibleMoves << new QGraphicsRectItem(posField.x()*m_nGridSize,
                                               posField.y()*m_nGridSize,
                                               m_nGridSize,
                                               m_nGridSize);
    listPossibleMoves.last()->setBrush(
          QBrush(m_pSettings->GetNeighboursColor()));
    listPossibleMoves.last()->setPen(
          QPen(m_pSettings->GetNeighboursBorderColor()));
    listPossibleMoves.last()->setVisible(true);
    this->addItem(listPossibleMoves.last());
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

quint8 Board::findPossibleMoves(const bool bStonesLeft) {
  // Return: 0 = no moves
  // 1 = stone can be set
  // 2 = tower can be moved
  // 3 = stone can be set and tower can be moved
  quint8 nRet(0);

  for (int y = 0; y < m_NumOfFields.y(); y++) {
    for (int x = 0; x < m_NumOfFields.x(); x++) {
      if (0 == m_Fields[x][y].size() && bStonesLeft && 1 != nRet) {
        nRet++;
      }
      if (m_Fields[x][y].size() > 0 && 2 != nRet) {
        if (this->checkNeighbourhood(QPoint(x, y)).size() > 0) {
          nRet += 2;
        }
      }
      if (3 == nRet) {
        return nRet;
      }
    }
  }
  return nRet;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::printDebugFields() const {
  qDebug() << "BOARD:";
  QString sLine;
  for (int nRow = 0; nRow < m_NumOfFields.y(); nRow++) {
    sLine.clear();
    for (int nCol = 0; nCol < m_NumOfFields.x(); nCol++) {
      sLine += "(";
      for (int tower = 0; tower < m_Fields[nCol][nRow].size(); tower++) {
        sLine += QString::number(m_Fields[nCol][nRow].at(tower));
        if (tower < m_Fields[nCol][nRow].size()-1) {
          sLine += ", ";
        }
      }
      sLine += ")";
      if (nCol < m_NumOfFields.x()-1) {
        sLine += " ";
      }
    }
    qDebug() << sLine;
  }
}
