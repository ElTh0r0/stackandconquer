/**
 * \file board.h
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

class Settings;

/**
 * \class Board
 * \brief Game board generation.
 */
class Board : public QGraphicsScene {
  Q_OBJECT

 public:
  Board(QWidget *pParent, const quint8 nMaxTower, quint8 NumOfPlayers,
        const QString &sIN, const QString &sOUT, Settings *pSettings,
        QObject *pParentObj = nullptr);

  auto createBoard(const QString &sBoard) -> bool;
  auto setupSavegame(const QJsonArray &jsBoard) -> bool;
  void addStone(const int nIndex, const quint8 nStone, const bool bAnim = true);
  void removeStone(const int nIndex, const bool bAll = false);
  void selectIndexField(const int nIndex);
  auto getBoard() const -> QJsonArray;
  auto getBoardDimensions() const -> QPoint;
  auto getMaxPlayerStones() const -> quint8;
  auto getField(const int index) const -> QString;
  auto getLegalMoves(const QString &sID, const bool bStonesLeft,
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

 public slots:
  void changeZoom();

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *p_Event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *p_Event) override;

 private slots:
  void resetAnimation();
  void resetAnimation2();

 private:
  auto loadBoard(const QString &sBoard, QList<QString> &tmpBoard) -> bool;
  void addBoardPadding(const QList<QString> &tmpBoard, const quint8 nMaxTower);
  void drawBoard(const QList<QString> &tmpBoard);
  void createHighlighters();
  auto createStones() -> bool;
  void startAnimation(const QPoint field);
  void startAnimation2(const QPoint field);
  auto snapToGrid(const QPointF point) const -> QPointF;
  void highlightNeighbourhood(const QList<int> &neighbours);
  auto getIndexFromField(const int nField) const -> int;
  auto getFieldFromIndex(const int nIndex) const -> int;

  QWidget *m_pParent;
  const QString m_sIN;
  const QString m_sOUT;
  const QString m_sPAD;
  QPoint m_BoardDimensions;
  QJsonArray m_jsBoard;

  QList<QGraphicsRectItem *> m_listFields;
  QPainterPath m_boardPath;

  Settings *m_pSettings;
  quint16 m_nGridSize;
  qreal m_nScale;
  quint8 m_nMaxPlayerStones;
  const quint8 m_nMaxTower;
  const quint8 m_NumOfPlayers;
  QList<int> m_DIRS;
  QGraphicsRectItem *m_pHighlightRect{};
  QGraphicsRectItem *m_pSelectedField{};
  QGraphicsRectItem *m_pAnimateField{};
  QGraphicsRectItem *m_pAnimateField2{};
  QList<QList<QGraphicsSvgItem *>> m_listPlayerStones;
  QList<QList<QGraphicsSvgItem *>> m_FieldStones;
  QList<QGraphicsSimpleTextItem *> m_Captions;
};

#endif  // BOARD_H_
