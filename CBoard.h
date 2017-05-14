/**
 * \file CBoard.h
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2017 Thorsten Roth <elthoro@gmx.de>
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
  CBoard(quint8 nNumOfFields, quint16 nGridSize, quint8 nMaxStones,
         CSettings *pSettings);

  void addStone(const QPoint field, const quint8 stone);
  void removeStone(const QPoint field, const bool bAll = false);
  void selectField(const QPointF point);
  QList<QList<QList<quint8> > > getBoard() const;
  QList<quint8> getField(const QPoint field) const;
  bool findPossibleMoves(const bool bStonesLeft);
  QList<QPoint> checkNeighbourhood(const QPoint field) const;
  void printDebugFields() const;

 signals:
  void setStone(QPoint);
  void moveTower(QPoint tower, QPoint moveTo);

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *p_Event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event);

 private slots:
  void resetAnimation();
  void resetAnimation2();

 private:
  void drawBoard();
  void createHighlighters();
  void createStones();
  void startAnimation(const QPoint field);
  void startAnimation2(const QPoint field);
  QPointF snapToGrid(const QPointF point) const;
  QPoint getGridField(const QPointF point) const;
  void highlightNeighbourhood(const QList<QPoint> neighbours);

  const quint16 m_nGridSize;
  const quint8 m_nMaxStones;
  CSettings *m_pSettings;
  const quint8 m_numOfFields;
  QRect m_BoardRect;
  QGraphicsRectItem *m_pHighlightRect;
  QGraphicsRectItem *m_pSelectedField;
  QGraphicsRectItem *m_pAnimateField;
  QGraphicsRectItem *m_pAnimateField2;
  QSvgRenderer *m_pSvgRenderer;
  QList<QGraphicsSvgItem *> m_listStonesP1;
  QList<QGraphicsSvgItem *> m_listStonesP2;

  QList<QList<QList<quint8> > > m_Fields;
  QList<QList<QList<QGraphicsSvgItem *> > > m_FieldStones;

  QList<QGraphicsSimpleTextItem *> m_FieldCaptions;
};

#endif  // STACKANDCONQUER_CBOARD_H_
