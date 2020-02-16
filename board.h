/**
 * \file board.h
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
 * Class definition for a board.
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QPolygonF>

#include <./settings.h>

/**
 * \class Board
 * \brief Game board generation.
 */
class Board : public QGraphicsScene {
  Q_OBJECT

 public:
    Board(QPoint NumOfFields, quint16 nGridSize, quint8 nMaxStones,
          Settings *pSettings);

    void setupSavegame(const QList<QList<QList<quint8> > > &board);
    void addStone(const QPoint field, const quint8 stone,
                  const bool bAnim = true);
    void removeStone(const QPoint field, const bool bAll = false);
    void selectField(const QPointF point);
    auto getBoard() const -> QList<QList<QList<quint8> > >;
    auto getField(const QPoint field) const -> QList<quint8>;
    auto findPossibleMoves(const bool bStonesLeft) -> quint8;
    auto checkNeighbourhood(const QPoint field) const -> QList<QPoint>;
    void printDebugFields() const;

 signals:
    void setStone(QPoint);
    void moveTower(QPoint tower, QPoint moveTo, quint8 nStones);

 protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *p_Event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event);

 private slots:
    void resetAnimation();
    void resetAnimation2();

 private:
    auto loadBoard(const QString &sBoard) -> bool;
    void drawBoard();
    void createHighlighters();
    void createStones();
    void startAnimation(const QPoint field);
    void startAnimation2(const QPoint field);
    auto snapToGrid(const QPointF point) const -> QPointF;
    auto getGridField(const QPointF point) const -> QPoint;
    void highlightNeighbourhood(const QList<QPoint> &neighbours);

    QList<QString> m_Board;
    const QString sIN;
    const QString sOUT;
    QPoint m_BoardDimension;

    QList<QGraphicsRectItem *> m_listFields;
    QPainterPath m_boardPath;

    const quint16 m_nGridSize;
    const quint8 m_nMaxStones;
    Settings *m_pSettings;
    const QPoint m_NumOfFields;
    QGraphicsRectItem *m_pHighlightRect{};
    QGraphicsRectItem *m_pSelectedField{};
    QGraphicsRectItem *m_pAnimateField{};
    QGraphicsRectItem *m_pAnimateField2{};
    QSvgRenderer *m_pSvgRenderer;
    QList<QGraphicsSvgItem *> m_listStonesP1;
    QList<QGraphicsSvgItem *> m_listStonesP2;

    QList<QList<QList<quint8> > > m_Fields;
    QList<QList<QList<QGraphicsSvgItem *> > > m_FieldStones;

    QList<QGraphicsSimpleTextItem *> m_Captions;
};

#endif  // BOARD_H_
