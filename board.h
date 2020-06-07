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
#include <QJsonArray>
#include <QList>
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
          const quint8 nMaxTower, quint8 NumOfPlayers, Settings *pSettings);

    void setupSavegame(const QList<QList<QList<quint8> > > &board);
    void addStone(const int nIndex, const quint8 nStone,
                  const bool bAnim = true);
    void removeStone(const int nIndex, const bool bAll = false);
    void selectIndexField(const int nIndex);
    auto getBoard() const -> QJsonArray;
    auto getField(const int index) const -> QString;
    auto findPossibleMoves(const bool bStonesLeft) -> quint8;
    auto checkNeighbourhood(const int nIndex) const -> QList<int>;
    void printDebugFields() const;
    // TODO(): Check if all variants are needed:
    auto getCoordinateFromField(const int nField) const -> QPoint;
    auto getStringCoordFromField(const int nField) const -> QString;
    auto getCoordinateFromIndex(const int nIndex) const -> QPoint;
    auto getStringCoordFromIndex(const int nIndex) const -> QString;

 signals:
    void setStone(int nIndex, bool bDebug);
    void moveTower(int nFrom, quint8 nStones, int nTo);

 protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *p_Event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event);

 private slots:
    void resetAnimation();
    void resetAnimation2();

 private:
    void loadBoard(const QString &sBoard, QList<QString> &tmpBoard);
    void addBoardPadding(const QList<QString> &tmpBoard,
                         const quint8 nMaxTower);
    void drawBoard(const QList<QString> &tmpBoard);
    void createHighlighters();
    void createStones();
    void startAnimation(const QPoint field);
    void startAnimation2(const QPoint field);
    auto snapToGrid(const QPointF point) const -> QPointF;
    void highlightNeighbourhood(const QList<int> &neighbours);
    auto getIndexFromField(const int nField) const -> int;
    auto getFieldFromIndex(const int nIndex) const -> int;

    const QString sIN;
    const QString sOUT;
    const QString sPAD;
    QPoint m_BoardDimension;
    QJsonArray m_jsBoard;

    QList<QGraphicsRectItem *> m_listFields;
    QPainterPath m_boardPath;

    const quint16 m_nGridSize;
    const quint8 m_nMaxStones;
    Settings *m_pSettings;
    const QPoint m_NumOfFields;  // TODO(): To be removed
    const quint8 m_nMaxTower;
    quint8 m_NumOfPlayers;
    QList<int> m_DIRS;
    QGraphicsRectItem *m_pHighlightRect{};
    QGraphicsRectItem *m_pSelectedField{};
    QGraphicsRectItem *m_pAnimateField{};
    QGraphicsRectItem *m_pAnimateField2{};
    QSvgRenderer *m_pSvgRendererP1;
    QSvgRenderer *m_pSvgRendererP2;
    QList<QGraphicsSvgItem *> m_listStonesP1;
    QList<QGraphicsSvgItem *> m_listStonesP2;
    QList<QList<QGraphicsSvgItem *> > m_FieldStones;
    QList<QGraphicsSimpleTextItem *> m_Captions;
};

#endif  // BOARD_H_
