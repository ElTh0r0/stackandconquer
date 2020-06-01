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

Board::Board(QPoint NumOfFields, quint16 nGridSize, quint8 nMaxStones,
             const quint8 nMaxTower, quint8 NumOfPlayers, Settings *pSettings)
  : sIN("0"),
    sOUT("#"),
    sPAD("-"),
    m_nGridSize(nGridSize),
    m_nMaxStones(nMaxStones),
    m_pSettings(pSettings),
    m_NumOfFields(NumOfFields),
    m_nMaxTower(nMaxTower),
    m_NumOfPlayers(NumOfPlayers),
    m_pSvgRendererP1(nullptr),
    m_pSvgRendererP2(nullptr) {
  this->setBackgroundBrush(QBrush(m_pSettings->getBgColor()));

  QList<QString> tmpBoard;
  this->loadBoard("./new_square_5x5.stackboard", tmpBoard);
  //this->loadBoard("./new_triangle.stackboard", tmpBoard);
  //this->loadBoard("./new_square_4x2.stackboard", tmpBoard);

  this->addBoardPadding(tmpBoard, m_nMaxTower);
  this->drawBoard(tmpBoard);
  this->createHighlighters();
  this->createStones();


  // Generate field matrix
  /*
   * Attention: For genrating a matrix m_Fields[nCol][nRow] the below logic
   * for generating the board is "column by column" and NOT "row by row":
   * m_Fields[nCol] "contains" nRow x elements
   */
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

  m_FieldStones2.clear();
  m_FieldStones2.reserve(m_jsBoard.size());
  QList<QGraphicsSvgItem *> tmpTower;
  for (int i = 0; i < m_jsBoard.size(); i++) {
    m_FieldStones2.append(tmpTower);
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::loadBoard(const QString &sBoard, QList<QString> &tmpBoard) {
  QFile fBoard(sBoard);
  if (!fBoard.exists()) {
    qWarning() << "Board cannot be loaded:" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return;
  }

  if (!fBoard.open(QIODevice::ReadOnly)) {
    qWarning() << "Couldn't open open board file:" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return;
  }

  QByteArray boardData = fBoard.readAll();
  QJsonDocument loadDoc(QJsonDocument::fromJson(boardData));
  QJsonObject jso = loadDoc.object();
  if (loadDoc.isEmpty() ||
      jso.value("Board").isUndefined() || !jso.value("Board").isArray() ||
      jso.value("Columns").isUndefined() || !jso.value("Columns").isDouble() ||
      jso.value("Rows").isUndefined() || !jso.value("Rows").isDouble()) {
    // TODO(): Extent check for all neeeded sections
    qWarning() << "Board file doesn't contain all required sections!" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return;
  }

  m_BoardDimension.setX(jso.value("Columns").toInt());
  m_BoardDimension.setY(jso.value("Rows").toInt());

  if (0 == m_BoardDimension.x() || 0 == m_BoardDimension.y()) {
    qWarning() << "Board file contains invalid dimension:" << m_BoardDimension;
    qWarning() << "Board:" << sBoard;
    QMessageBox::critical(nullptr, tr("Warning"),
                         tr("Error while opening board file!"));
    return;
  }

  tmpBoard.clear();
  foreach (QJsonValue js, jso.value("Board").toArray()) {
    QString s = js.toString();
    if (js.isNull() || s.isEmpty() || (sOUT != s && sIN != s && sPAD != s)) {
      qWarning() << "Board array contains invalid data:" << s;
      qWarning() << "Board:" << sBoard;
      QMessageBox::critical(nullptr, tr("Warning"),
                           tr("Error while opening board file!"));
      return;
    }
    tmpBoard << s;
  }



  // TODO(): Add all fields from json file

  qDebug() << "Board dimensions:" << m_BoardDimension.x() << "columns x"
           << m_BoardDimension.y() << "rows";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::addBoardPadding(const QList<QString> &tmpBoard,
                            const quint8 nMaxTower) {
  // Generate field array
  // Top padding
  for (int i = 0; i < nMaxTower; i++) {
    for (int j = 0; j < (nMaxTower*2 + m_BoardDimension.x()); j++) {
      m_jsBoard << sPAD;
    }
  }

  // Padding left and right per line
  for (int i = 0; i < tmpBoard.size(); i++) {
    if (0 == i) {  // First item of first line, padding left
      for (int j = 0; j < nMaxTower; j++) {
        m_jsBoard << sPAD;
      }
    } else if (0 == i % m_BoardDimension.x()) {  // First item in row:
      for (int j = 0; j < (nMaxTower*2); j++) {  // Add padding end of previous
        m_jsBoard << sPAD;                       // & beginning of current line
      }
    }

    if (sIN == tmpBoard[i]) {
      m_jsBoard << QString();
    } else {
      m_jsBoard << sOUT;
    }

    if (tmpBoard.size()-1 == i) {  // Last item of last line, padding right
      for (int j = 0; j < nMaxTower; j++) {
        m_jsBoard << sPAD;
      }
    }
  }

  // Bottom padding
  for (int i = 0; i < nMaxTower; i++) {
    for (int j = 0; j < (nMaxTower*2 + m_BoardDimension.x()); j++) {
      m_jsBoard << sPAD;
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
    // m_BoardDimension.x() = columns, m_BoardDimension.y() = rows
    if (0 == i % m_BoardDimension.x()) {  // First item in row
      x = 0;
      col = 65;
      y += m_nGridSize;
      row++;
    } else {
      x += m_nGridSize;
      col++;
    }
    if (sOUT == tmpBoard[i]) {
      m_listFields << new QGraphicsRectItem();
      continue;
    }

    m_listFields << new QGraphicsRectItem(x, y, m_nGridSize, m_nGridSize);
    m_listFields.last()->setBrush(QBrush(m_pSettings->getBgBoardColor()));
    m_listFields.last()->setPen(QPen(m_pSettings->getGridBoardColor()));
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
  // Load svg as txt for below color exchange.
  QFile fStone(QStringLiteral(":/images/stone.svg"));
  if (!fStone.open(QFile::ReadOnly | QFile::Text)) {
    qDebug() << "Could not open stone.svg";
    QMessageBox::critical(nullptr, tr("Warning"),
                          tr("Could not open %1!").arg("stone.svg"));
    return;
  }
  QTextStream in(&fStone);
  QString sSvg = in.readAll();
  fStone.close();

  // stone.svg HAS to be filled with #ff0000, so that below replace can work.
  QString sTmpSvg = sSvg;
  QByteArray aSvg1(sTmpSvg.replace(
                     "#ff0000", m_pSettings->getPlayerColor(1)).toUtf8());
  sTmpSvg = sSvg;
  QByteArray aSvg2(sTmpSvg.replace("#ff0000",
                                   m_pSettings->getPlayerColor(2)).toUtf8());

  // TODO(): Create dynamic list to support >2 players.
  auto *m_pSvgRendererP1 = new QSvgRenderer(aSvg1);
  auto *m_pSvgRendererP2 = new QSvgRenderer(aSvg2);

  // Create a few more than maximum of stones because of wrong
  // order during move tower add/remove
  for (int i = 0; i < m_nMaxStones + 4; i++) {
    m_listStonesP1.append(new QGraphicsSvgItem());
    // Don't transform graphics to isometric view!
    m_listStonesP1.last()->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    m_listStonesP1.last()->setSharedRenderer(m_pSvgRendererP1);
    this->addItem(m_listStonesP1.last());
    m_listStonesP1.last()->setPos(0, 0);
    m_listStonesP1.last()->setZValue(5);
    m_listStonesP1.last()->setVisible(false);

    m_listStonesP2.append(new QGraphicsSvgItem());
    // Don't transform graphics to isometric view!
    m_listStonesP2.last()->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    m_listStonesP2.last()->setSharedRenderer(m_pSvgRendererP2);
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
        // TODO(): Implement new board array
        //this->addStone(QPoint(nCol, nRow), stone);
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
    QList<QGraphicsItem *> items = this->items(p_Event->scenePos());
    foreach (auto item, items) {
      int field = m_listFields.indexOf(
                    qgraphicsitem_cast<QGraphicsRectItem *>(item));
      if (field > -1) {
        qDebug() << "Clicked field:" << field;
        qDebug() << "Board index:  " << getIndexFromField(field);
        qDebug() << "Coordinate:   " << getCoordinateFromField(field);
        qDebug() << "String coord.:" << getStringCoordFromField(field);

        // Place tower, if field is empty
        if (m_jsBoard.at(getIndexFromField(field)).toString().isEmpty()) {
          qDebug() << "FIELD EMPTY";
          this->selectIndexField(-1);
          emit setStone(getIndexFromField(field));
        } else {  // Otherwise select / move tower
          qDebug() << "FIELD not EMPTY --> SELECT";
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
    point = QPointF(point.x() - m_nGridSize/2, point.y() - m_nGridSize/2);
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
  static const int nTop = m_nMaxTower * (2*m_nMaxTower + m_BoardDimension.x());
  static const int nFirst = nTop + m_nMaxTower;
  int nLeftRight = 2*m_nMaxTower * (nField / m_BoardDimension.x());
  int nCol = nField % m_BoardDimension.x();
  int nRow = (nField / m_BoardDimension.x()) * m_BoardDimension.x();
  return nFirst + nLeftRight + nCol + nRow;
}

auto Board::getFieldFromIndex(const int nIndex) const -> int {
  static const int nTop = m_nMaxTower * (2*m_nMaxTower + m_BoardDimension.x());
  static const int nFirst = nTop + m_nMaxTower;
  int nLeftRight = 2*m_nMaxTower *
                   ((nIndex/(m_BoardDimension.x()+2*m_nMaxTower))-m_nMaxTower);
  return nIndex - nFirst - nLeftRight;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getCoordinateFromField(const int nField) const -> QPoint {
  int x = nField % m_BoardDimension.x();
  int y = nField / m_BoardDimension.x();
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
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
    return;
  }

  m_jsBoard[nIndex] = m_jsBoard[nIndex].toString() + QString::number(nStone);
  // TODO(): Rewrite for dynamic stone generation and > 2 players
  if (1 == nStone) {
    m_FieldStones2[nIndex].append(m_listStonesP1.last());
    m_listStonesP1.removeLast();
  } else if (2 == nStone) {
    m_FieldStones2[nIndex].append(m_listStonesP2.last());
    m_listStonesP2.removeLast();
  }

  if (bAnim) {
    this->startAnimation(this->getCoordinateFromIndex(nIndex));
  }

  // TODO(): Make position dynamic depening on stone size
  m_FieldStones2[nIndex].last()->setPos(
        this->getCoordinateFromIndex(nIndex)*m_nGridSize);
  m_FieldStones2[nIndex].last()->setPos(
        m_FieldStones2[nIndex].last()->x() - 16 - 13*nExisting,
      m_FieldStones2[nIndex].last()->y() + 20 - 13*nExisting);
  m_FieldStones2[nIndex].last()->setVisible(true);

  for (int z = 0; z < m_FieldStones2.at(nIndex).size(); z++) {
    m_FieldStones2[nIndex][z] ->setZValue(6 + z);
  }

  if (bAnim) {
    // Redraw board
    this->update(QRectF(0, 0, m_BoardDimension.x() * m_nGridSize-1,
                        m_BoardDimension.y() * m_nGridSize-1));
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
  if (m_Fields[field.x()][field.y()].isEmpty()) {
    qWarning() << "Trying to remove stone from empty field" << field;
    QMessageBox::warning(nullptr, tr("Warning"), tr("Something went wrong!"));
    return;
  }

  if (bAll) {  // Remove all (tower conquered)
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
  this->update(QRectF(0, 0, m_BoardDimension.x() * m_nGridSize-1,
                      m_BoardDimension.y() * m_nGridSize-1));
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getBoard() const -> QList<QList<QList<quint8> > > {
  return m_Fields;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::getField(const int index) const -> QString {
  return m_jsBoard.at(index).toString();
}

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

  if (currentIndex == nIndex ||
      m_jsBoard.at(nIndex).toString().isEmpty()) {
    currentIndex = -1;
    m_pSelectedField->setVisible(false);
    this->highlightNeighbourhood(neighbours);
    // qDebug() << "Deselected";
    return;
  }

  neighbours = this->checkNeighbourhood(currentIndex);
  if (neighbours.contains(nIndex) && m_pSelectedField->isVisible()) { // Move
    neighbours.clear();
    this->highlightNeighbourhood(neighbours);
    m_pSelectedField->setVisible(false);
    this->startAnimation2(this->getCoordinateFromIndex(nIndex));
//    emit moveTower(nIndex, currentIndex, 0);
    currentIndex = -1;
  } else {  // Select
    currentIndex = nIndex;
    m_pSelectedField->setVisible(true);
    m_pSelectedField->setRect(m_listFields.at(
                                this->getFieldFromIndex(nIndex))->rect());
    if (m_pSettings->getShowPossibleMoveTowers()) {
      this->highlightNeighbourhood(
            this->checkNeighbourhood(currentIndex));
    }
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::checkNeighbourhood(const int field) const -> QList<int> {
  return QList<int>();
  /*
  QList<QPoint> neighbours;
  if (QPoint(-1, -1) == field) {
    return neighbours;
  }

  auto nMoves = static_cast<quint8>(m_Fields[field.x()][field.y()].size());
  // qDebug() << "Selected:" << field << "- Moves:" << nMoves;

  for (int y = field.y() - nMoves; y <= field.y() + nMoves; y += nMoves) {
    for (int x = field.x() - nMoves; x <= field.x() + nMoves; x += nMoves) {
      if (x < 0 || y < 0 || x >= m_BoardDimension.x() || y >= m_BoardDimension.y() ||
          field == QPoint(x, y)) {
        continue;
      }
      if (!m_Fields[x][y].isEmpty()) {
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
          if (!m_Fields[check.x()][check.y()].isEmpty()) {
            // qDebug() << "Route blocked";
            bBreak = true;
            break;
          }
        }

        if (!bBreak) {
          neighbours.append(QPoint(x, y));
        }
      }
    }
  }
  // qDebug() << "Neighbours which can be moved:" << neighbours;
  return neighbours;
  */
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::highlightNeighbourhood(const QList<int> &neighbours) {
/*
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
          QBrush(m_pSettings->getNeighboursColor()));
    listPossibleMoves.last()->setPen(
          QPen(m_pSettings->getNeighboursBorderColor()));
    listPossibleMoves.last()->setVisible(true);
    this->addItem(listPossibleMoves.last());
  }
*/
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto Board::findPossibleMoves(const bool bStonesLeft) -> quint8 {
  // Return: 0 = no moves
  // 1 = stone can be set
  // 2 = tower can be moved
  // 3 = stone can be set and tower can be moved
return 3;
  /*
  quint8 nRet(0);
  for (int y = 0; y < m_BoardDimension.y(); y++) {
    for (int x = 0; x < m_BoardDimension.x(); x++) {
      if (m_Fields[x][y].isEmpty() && bStonesLeft && 1 != nRet) {
        nRet++;
      }
      if (!m_Fields[x][y].isEmpty() && 2 != nRet) {
        if (!this->checkNeighbourhood(QPoint(x, y)).isEmpty()) {
          nRet += 2;
        }
      }
      if (3 == nRet) {
        return nRet;
      }
    }
  }
  return nRet;
*/
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void Board::printDebugFields() const {
  qDebug() << "BOARD:";
  QString sLine;
  int nField = -1;
  QString s;

  for (int nRow = 0; nRow < m_BoardDimension.y(); nRow++) {
    sLine.clear();
    for (int nCol = 0; nCol < m_BoardDimension.x(); nCol++) {
      nField++;
      s = "(" + m_jsBoard.at(this->getIndexFromField(nField)).toString() + ")";
      if (QString("(" + sOUT + ")") == s) { s = sOUT + sOUT; }
      sLine += s;
      if (nCol < m_BoardDimension.x()-1) {
        sLine += " ";
      }
    }
    qDebug() << sLine;
  }
}
