/**
 * \file CBoard.cpp
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
 * Game board generation.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>

#include "./CBoard.h"

CBoard::CBoard(QGraphicsView *pGraphView, quint16 nGridSize, quint8 nMaxStones)
    : m_pGraphView(pGraphView),
      m_nGridSize(nGridSize),
      m_nMaxStones(nMaxStones),
      m_numOfFields(5),
      m_pSvgRenderer(NULL) {
    qDebug() << Q_FUNC_INFO;
    this->setBackgroundBrush(QBrush(QColor("#EEEEEC")));
    this->drawGrid();

    // Field highlighter
    m_pHighlightRect = new QGraphicsRectItem(0, 0, m_nGridSize, m_nGridSize);
    m_pHighlightRect->setBrush(QBrush(QColor("#8ae234")));
    m_pHighlightRect->setPen(QPen(QColor("#888A85")));
    m_pHighlightRect->setVisible(false);
    m_pHighlightRect->setZValue(0);
    this->addItem(m_pHighlightRect);
    // Selected field
    m_pSelectedField = new QGraphicsRectItem(0, 0, m_nGridSize, m_nGridSize);
    m_pSelectedField->setBrush(QBrush(QColor("#fce94f")));
    m_pSelectedField->setPen(QPen(QColor("#000000")));
    m_pSelectedField->setVisible(false);
    m_pSelectedField->setZValue(0);
    this->addItem(m_pSelectedField);

    // Generate stones
    QSvgRenderer *m_pSvgRenderer = new QSvgRenderer(
                QLatin1String(":/images/stones.svg"));
    // Create a few more than maximum of stones because of wrong
    // order during move tower add/remove
    for (int i = 0; i < m_nMaxStones + 4; i++) {
        m_listStonesP1.append(new QGraphicsSvgItem());
        // Don't transform graphics to isometric view!
        m_listStonesP1.last()->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        m_listStonesP1.last()->setSharedRenderer(m_pSvgRenderer);
        m_listStonesP1.last()->setElementId(QLatin1String("Stone1"));
        this->addItem(m_listStonesP1.last());
        m_listStonesP1.last()->setPos(0, 0);
        m_listStonesP1.last()->setZValue(5);
        m_listStonesP1.last()->setVisible(false);

        m_listStonesP2.append(new QGraphicsSvgItem());
        // Don't transform graphics to isometric view!
        m_listStonesP2.last()->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        m_listStonesP2.last()->setSharedRenderer(m_pSvgRenderer);
        m_listStonesP2.last()->setElementId(QLatin1String("Stone2"));
        this->addItem(m_listStonesP2.last());
        m_listStonesP2.last()->setPos(0, 0);
        m_listStonesP2.last()->setZValue(5);
        m_listStonesP2.last()->setVisible(false);
    }

    // Generate field matrix
    QList<quint8> tower;
    QList<QList<quint8> > line;
    QList<QGraphicsSvgItem *> tower2;
    QList<QList<QGraphicsSvgItem *> > line2;
    for (int i = 0; i < m_numOfFields; i++) {
        line.append(tower);
        line2.append(tower2);
    }
    m_Fields.clear();
    m_FieldStones.clear();
    for (int i = 0; i < m_numOfFields; i++) {
        m_Fields.append(line);
        m_FieldStones.append(line2);
    }
    // qDebug() << "FIELDS:" << m_Fields;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::drawGrid() {
    qDebug() << Q_FUNC_INFO;
    m_BoardRect.setTopLeft(QPoint(0, 0));
    m_BoardRect.setBottomRight(QPoint(m_numOfFields * m_nGridSize -1,
                                      m_numOfFields * m_nGridSize -1));

    // Draw board
    QPen linePen(QColor("#2E3436"));
    this->addRect(m_BoardRect, linePen, QBrush(QColor("#FFFFFF")));
    m_pGraphView->setSceneRect(m_BoardRect);

    // Draw lines
    QLineF lineGrid;
    linePen.setColor(QColor("#888A85"));
    // Horizontal
    for (int i = 1; i < m_numOfFields; i++) {
        lineGrid.setPoints(QPointF(1, i*m_nGridSize),
                           QPointF(m_BoardRect.width()-1, i*m_nGridSize));
        this->addLine(lineGrid, linePen);
    }
    // Vertical
    for (int i = 1; i < m_numOfFields; i++) {
        lineGrid.setPoints(QPointF(i*m_nGridSize, 1),
                           QPointF(i*m_nGridSize, m_BoardRect.height()-1));
        this->addLine(lineGrid, linePen);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::mousePressEvent(QGraphicsSceneMouseEvent *p_Event) {
    // Check, if mouse is inside the board rectangle (+1/-1 for border line)
    static QRectF board(m_BoardRect.topLeft() + QPoint(1, 1),
                        m_BoardRect.bottomRight() - QPoint(1, 1));

    if (board.contains(p_Event->scenePos())) {
        // qDebug() << "Mouse POS:" << p_Event->scenePos();
        // qDebug() << "SNAP:" << this->snapToGrid(p_Event->scenePos());
        // qDebug() << "GRID:" << this->getGridField(p_Event->scenePos());

        if (p_Event->button() == Qt::LeftButton) {
            this->selectField(p_Event->scenePos());
        } else {
            selectField(QPointF(-1, -1));
            emit setStone(this->getGridField(p_Event->scenePos()));
        }
    }

    QGraphicsScene::mousePressEvent(p_Event);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event) {
    static QPointF point;
    // Check, if mouse is inside the board rectangle (+1/-1 for border line)
    static QRectF board(m_BoardRect.topLeft() + QPoint(1, 1),
                        m_BoardRect.bottomRight() - QPoint(1, 1));

    if (board.contains(p_Event->scenePos())) {
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

QPointF CBoard::snapToGrid(const QPointF point) const {
    int x = qRound(point.x() / m_nGridSize) * m_nGridSize;
    int y = qRound(point.y() / m_nGridSize) * m_nGridSize;
    return QPointF(x, y);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QPoint CBoard::getGridField(QPointF point) const {
    qint8 x = point.toPoint().x() / m_nGridSize;
    qint8 y = point.toPoint().y() / m_nGridSize;

    if (x < 0 || y < 0 || x >= m_numOfFields || y >= m_numOfFields) {
        qWarning() << "Point out of grid! (" << x << "," << y << ")";
        if (x < 0) { x = 0; }
        if (y < 0) { y = 0; }
        if (x >= m_numOfFields) { x = m_numOfFields - 1; }
        if (y >= m_numOfFields) { y = m_numOfFields - 1; }
        qWarning() << "Changed point to (" << x << "," << y << ")";
    }

    return QPoint(x, y);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::addStone(QPoint field, quint8 stone) {
    quint8 nExisting = m_Fields[field.x()][field.y()].size();

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
        QMessageBox::warning(NULL, "Warning", "Something went wrong!");
        return;
    }

    m_FieldStones[field.x()][field.y()].last()->setPos(field*m_nGridSize);
    m_FieldStones[field.x()][field.y()].last()->setPos(
            m_FieldStones[field.x()][field.y()].last()->x() - 16 - 13*nExisting,
            m_FieldStones[field.x()][field.y()].last()->y() + 20 - 13*nExisting);
    m_FieldStones[field.x()][field.y()].last()->setVisible(true);

    for (int z = 0; z < m_FieldStones[field.x()][field.y()].size(); z++) {
        m_FieldStones[field.x()][field.y()][z]->setZValue(6 + z);
    }

    qDebug() << "FIELDS";
    for (int i = 0; i < m_numOfFields; i++) {
        qDebug() << m_Fields[0][i] << m_Fields[1][i] << m_Fields[2][i]
                 << m_Fields[3][i] << m_Fields[4][i];
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::removeStone(QPoint field, bool bAll) {
    if (0 == m_Fields[field.x()][field.y()].size()) {
        qWarning() << "Trying to remove stone from empty field" << field;
        QMessageBox::warning(NULL, "Warning", "Something went wrong!");
        return;
    } else if (bAll) {
        foreach (quint8 i, m_Fields[field.x()][field.y()]) {
            if (1 == i) {
                m_listStonesP1.append(m_FieldStones[field.x()][field.y()].last());
                m_FieldStones[field.x()][field.y()].removeLast();
            } else {
                m_listStonesP2.append(m_FieldStones[field.x()][field.y()].last());
                m_FieldStones[field.x()][field.y()].removeLast();
            }
        }
        m_Fields[field.x()][field.y()].clear();
    } else {
        if (1 == m_Fields[field.x()][field.y()].last()) {
            m_listStonesP1.append(m_FieldStones[field.x()][field.y()].last());
            m_FieldStones[field.x()][field.y()].last()->setVisible(false);
            m_FieldStones[field.x()][field.y()].removeLast();
        } else {
            m_listStonesP2.append(m_FieldStones[field.x()][field.y()].last());
            m_FieldStones[field.x()][field.y()].last()->setVisible(false);
            m_FieldStones[field.x()][field.y()].removeLast();
        }
        m_Fields[field.x()][field.y()].removeLast();
    }

    qDebug() << "FIELDS";
    for (int i = 0; i < m_numOfFields; i++) {
        qDebug() << m_Fields[0][i] << m_Fields[1][i] << m_Fields[2][i]
                 << m_Fields[3][i] << m_Fields[4][i];
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QList<quint8> CBoard::getField(QPoint field) const {
    return m_Fields[field.x()][field.y()];
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::selectField(QPointF point) {
    static QPoint currentField(QPoint(-1, -1));
    QPointF pointSnap(point);
    pointSnap = QPointF(pointSnap.x() - m_nGridSize/2,
                        pointSnap.y() - m_nGridSize/2);
    pointSnap = this->snapToGrid(pointSnap);
    QList<QPoint> neighbours;

    // TODO: Optimize calls (e.g. deselect)
    if (QPointF(-1, -1) == point) {
        currentField = QPoint(-1, -1);
        m_pSelectedField->setVisible(false);
        this->highlightNeighbourhood(neighbours);
        qDebug() << "Deselected";
        return;
    } else {
        QPoint field = this->getGridField(point);
        if (currentField == field ||
                0 == m_Fields[field.x()][field.y()].size()) {
            currentField = QPoint(-1, -1);
            m_pSelectedField->setVisible(false);
            this->highlightNeighbourhood(neighbours);
            qDebug() << "Deselected";
            return;
        }
        neighbours = this->checkNeighbourhood(currentField);
        if (neighbours.contains(field) && m_pSelectedField->isVisible()) {
            neighbours.clear();
            this->highlightNeighbourhood(neighbours);
            m_pSelectedField->setVisible(false);
            emit moveTower(field ,currentField);
            currentField = QPoint(-1, -1);
        } else {
            currentField = field;
            m_pSelectedField->setVisible(true);
            m_pSelectedField->setPos(pointSnap);
            this->highlightNeighbourhood(this->checkNeighbourhood(currentField));
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

QList<QPoint> CBoard::checkNeighbourhood(QPoint field) {
    QList<QPoint> neighbours;
    if (QPoint(-1, -1) == field) {
        return neighbours;
    }

    quint8 nMoves = m_Fields[field.x()][field.y()].size();
    qDebug() << "Selected:" << field << "- Moves:" << nMoves;

    // TODO: Optimize loops...
    for (int y = field.y() - nMoves; y <= field.y() + nMoves; y += nMoves) {
        for (int x = field.x() - nMoves; x <= field.x() + nMoves; x += nMoves) {
            if (x < 0 || y < 0 || x >= m_numOfFields || y >= m_numOfFields
                    || field == QPoint(x, y)) {
                continue;
            } else if (m_Fields[x][y].size() > 0) {
                // Check for blocking towers in between
                QPoint check(x, y);
                qDebug() << "POSSIBLE:" << check;
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

                    qDebug() << "Check route:" << check;
                    if (m_Fields[check.x()][check.y()].size() > 0) {
                        qDebug() << "Route blocked";
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
    qDebug() << "Neighbours which can be moved:" << neighbours;
    return neighbours;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void CBoard::highlightNeighbourhood(QList<QPoint> neighbours) {
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
        listPossibleMoves.last()->setBrush(QBrush(QColor("#ad7fa8")));
        listPossibleMoves.last()->setPen(QPen(QColor("#000000")));
        listPossibleMoves.last()->setVisible(true);
        this->addItem(listPossibleMoves.last());
    }
}
