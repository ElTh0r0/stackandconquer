/**
 * \file board.h
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
 * Class definition for a board.
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <QGraphicsScene>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QPainterPath>

class QGraphicsSceneMouseEvent;
class QGraphicsSvgItem;
class QSvgRenderer;

class Settings;

/**
 * \class Board
 * \brief Game board generation.
 */
class Board : public QGraphicsScene {
  Q_OBJECT

 public:
    Board(const QString &sBoard, quint16 nGridSize, const quint8 nMaxTower,
          quint8 NumOfPlayers, Settings *pSettings);

    auto setupSavegame(const QJsonArray &jsBoard) -> bool;
    void addStone(const int nIndex, const quint8 nStone,
                  const bool bAnim = true);
    void removeStone(const int nIndex, const bool bAll = false);
    void selectIndexField(const int nIndex);
    auto getBoard() const -> QJsonArray;
    auto getBoadDimensions() const -> QPoint;
    auto getMaxPlayerStones() const -> quint8;
    auto getField(const int index) const -> QString;
    auto getLegalMoves(const bool bStonesLeft,
                       const QList<int> &lastMove) const -> QJsonDocument;
    auto checkNeighbourhood(const int nIndex) const -> QList<int>;
    void printDebugFields() const;
    auto getCoordinateFromField(const int nField) const -> QPoint;
    auto getStringCoordFromField(const int nField) const -> QString;
    auto getCoordinateFromIndex(const int nIndex) const -> QPoint;
    auto getStringCoordFromIndex(const int nIndex) const -> QString;
    auto getOut() const -> QString;
    auto getPad() const -> QString;

 signals:
    void actionPlayer(QJsonArray move);

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
    QPoint m_BoardDimensions;
    QJsonArray m_jsBoard;

    QList<QGraphicsRectItem *> m_listFields;
    QPainterPath m_boardPath;

    const quint16 m_nGridSize;
    quint8 m_nMaxPlayerStones;
    Settings *m_pSettings;
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
