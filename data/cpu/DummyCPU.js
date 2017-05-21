/**
 * \file DummyCPU.js
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
 * Dummy CPU opponent.
 *
 * Variables provided externally from game:
 * jsboard
 * nID (1 or 2 = player 1 / player 2)
 * nNumOfFields
 * nHeightTowerWin
 */

cpu.log("Loading CPU script DummyCPU...")

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function makeMove(bStonesLeft) {
  board = JSON.parse(jsboard);
  //cpu.log("[0][0][0]: " + board[0][0][0]);
  //cpu.log("[1][0].length: " + board[1][0].length);
  
  sMoveToWin = canWin(nID);
  if (0 !== sMoveToWin.length) {
    return sMoveToWin;
  }

  if (bStonesLeft) {
    if (findFreeFields()) {
      return setStone();
    } else {
      return moveTower();
    }
  } else {
    return moveTower();
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function setStone() {
  return setRandom();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function moveTower() {
  return "0,0|1,1|1";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function checkNeighbourhood(nFieldX, nFieldY)  {
  var neighbours = [];
  var nMoves = board[nFieldX][nFieldY].length

  if (0 === nMoves) {
    return neighbours;
  }

  for (var y = nFieldY - nMoves; y <= nFieldY + nMoves; y += nMoves) {
    for (var x = nFieldX - nMoves; x <= nFieldX + nMoves; x += nMoves) {
      if (x < 0 || y < 0 || x >= nNumOfFields || y >= nNumOfFields ||
          (nFieldX === x && nFieldY === y)) {
        continue;
      } else if (board[x][y].length > 0) {
        // Check for blocking towers in between
        var checkX = x;
        var checkY = y;
        var routeX = nFieldX - checkX;
        var routeY = nFieldY - checkY;
        var bBreak = false;

        for (var i = 1; i < nMoves; i++) {
          if (routeY < 0) {
            checkY = checkY - 1;
          } else if (routeY > 0) {
            checkY = checkY + 1;
          } else {
            checkY = y;
          }

          if (routeX < 0) {
            checkX = checkX - 1;
          } else if (routeX > 0) {
            checkX = checkX + 1;
          } else {
            checkX = x;
          }

          if (board[checkX][checkY].length > 0) {
            // Route blocked
            bBreak = true;
            break;
          }
        }

        if (false == bBreak) {
          var point = [x, y];
          neighbours.push(point);
        }
      }
    }
  }
  return neighbours;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function canWin(nPlayerID)  {
  for (var nRow = 0; nRow < nNumOfFields; nRow++) {
    for (var nCol = 0; nCol < nNumOfFields; nCol++) {
      neighbours = checkNeighbourhood(nRow, nCol);
      // cpu.log("Check: " + (nRow+1) + "," + (nCol+1));
      // cpu.log("Neighbours: " + neighbours);

      for (var point = 0; point < neighbours.length; point++) {
        var tower = board[(neighbours[point])[0]][(neighbours[point])[1]];
        if ((board[nRow][nCol].length + tower.length >= nHeightTowerWin) &&
            nPlayerID === tower[tower.length - 1]) {  // Top stone = own color
          return (neighbours[point])[0] + "," + (neighbours[point])[1] + "|" +
              nRow + "," + nCol + "|" + tower.length;
        }
      }
    }
  }
  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function setRandom() {
  // Seed random?
  do {
    nRandX = Math.floor(Math.random() * nNumOfFields);
    nRandY = Math.floor(Math.random() * nNumOfFields);
  } while (0 !== board[nRandX][nRandY].length);
  
  return nRandX + "," + nRandY;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function findFreeFields() {
  for (var nRow = 0; nRow < nNumOfFields; nRow++) {
    for (var nCol = 0; nCol < nNumOfFields; nCol++) {
      if (0 === board[nRow][nCol].length) {
        return true;
      }
    }
  }
  return false;
}
