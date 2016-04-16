/**
 * \file CBoard.h
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
 * Class definition for a board.
 */

#ifndef STACKANDCONQUER_CBOARD_H_
#define STACKANDCONQUER_CBOARD_H_

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QPolygonF>

#include <./CSettings.h>

/**
 * \class CBoard
 * \brief Game board generation.
 */
class CBoard : public QGraphicsScene {
    Q_OBJECT

  public:
    CBoard(QGraphicsView *pGraphView, quint16 nGridSize, quint8 nMaxStones,
           CSettings *pSettings);

    void addStone(QPoint field, quint8 stone);
    void removeStone(QPoint field, bool bAll = false);
    QList<quint8> getField(QPoint field) const;

  signals:
    void setStone(QPoint);
    void moveTower(QPoint tower, QPoint moveTo);

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *p_Event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event);

  private:
    QGraphicsView *m_pGraphView;
    const quint16 m_nGridSize;
    const quint8 m_nMaxStones;
    CSettings *m_pSettings;
    const quint8 m_numOfFields;
    QRect m_BoardRect;
    QGraphicsRectItem *m_pHighlightRect;
    QGraphicsRectItem *m_pSelectedField;
    QSvgRenderer *m_pSvgRenderer;
    QList<QGraphicsSvgItem *> m_listStonesP1;
    QList<QGraphicsSvgItem *> m_listStonesP2;

    QList<QList<QList<quint8> > > m_Fields;
    QList<QList<QList<QGraphicsSvgItem *> > > m_FieldStones;

    void drawGrid();
    QPointF snapToGrid(QPointF point) const;
    QPoint getGridField(QPointF point) const;
    void selectField(QPointF point);
    QList<QPoint> checkNeighbourhood(QPoint field);
    void highlightNeighbourhood(QList<QPoint> neighbours);
};

#endif  // STACKANDCONQUER_CBOARD_H_
