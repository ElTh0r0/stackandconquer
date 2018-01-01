/**
 * \file DummyCPU.js
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2018 Thorsten Roth <elthoro@gmx.de>
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

cpu.log("Loading CPU script DummyCPU...");

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function makeMove(nPossible) {
  board = JSON.parse(jsboard);  // Global
  nPossibleMove = Number(nPossible);
  //cpu.log("[0][0][0]: " + board[0][0][0]);
  //cpu.log("[1][0].length: " + board[1][0].length);
  
  var sMoveToWin = canWin(nID);
  if (0 !== sMoveToWin.length) {
    return sMoveToWin;
  }

  // Check if opponent can win
  if (2 === nID) {
    sMoveToWin = canWin(1);
  } else {
    sMoveToWin = canWin(2);
  }
  if (0 !== sMoveToWin.length) {
    var sPreventWin = preventWin(sMoveToWin, nPossibleMove);
    if (sPreventWin.length > 0) {
      return sPreventWin;
    }
  }

  if (1 === nPossibleMove && findFreeFields()) {
    return setStone();
  } else if (2 === nPossibleMove) {
    return moveTower();
  } else if (3 === nPossibleMove) {
    var nRand = Math.floor(Math.random() * 2);
    if (0 === nRand) {
      return setStone();
    } else {
      return moveTower();
    }
  }

  // This line never should be reached!
  cpu.log("ERROR: Script couldn't call setStone() / moveTower()!");
  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function setStone() {
  return setRandom();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function moveTower() {
  return moveRandom();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function checkNeighbourhood(nFieldX, nFieldY)  {
  var neighbours = [];
  var nMoves = board[nFieldX][nFieldY].length;

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
      var neighbours = checkNeighbourhood(nRow, nCol);
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

function preventWin(sMoveToWin, nPossibleMove) {
  var sMove = sMoveToWin.split("|");
  var pointFrom = sMove[0].split(",");
  var pointTo = sMove[1].split(",");
  var nNumber = sMove[2];

  // Check if a blocking towers in between can be placed
  var route = [Number(pointTo[0]) - Number(pointFrom[0]),
               Number(pointTo[1]) - Number(pointFrom[1])];
  var nMoves = board[pointTo[0]][pointTo[1]].length;
  var check = [Number(pointFrom[0]), Number(pointFrom[1])];

  // cpu.log("Win? " + sMoveToWin);
  // cpu.log("Route: " + route[0] + "," + route[1]);
  // cpu.log("Moves: " + nMoves);
  // cpu.log("Check 0: " + check[0] + "," + check[1]);
  // cpu.log("Possible: " + nPossibleMove);

  if (nMoves > 1 && (1 === nPossibleMove || 3 === nPossibleMove)) {
    for (var i = 1; i < nMoves; i++) {

      if (route[1] < 0) {
        check[1] = Number(check[1] - 1);
      } else if (route[1] > 0) {
        check[1] = Number(check[1] + 1);
      }

      if (route[0] < 0) {
        check[0] = Number(check[0] - 1);
      } else if (route[0] > 0) {
        check[0] = Number(check[0] + 1);
      }

      // cpu.log("Check " + i + ": " + check[0] + "," + check[1]);
      if (0 === board[check[0]][check[1]].length) {
        return check[0] + "," + check[1];
      }
    }
  }
  // if (nPossibleMove >= 2) {
  // TODO: Try to move tower to prevent win
  // }

  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function setRandom() {
  // Seed random?
  do {
    var nRandX = Math.floor(Math.random() * nNumOfFields);
    var nRandY = Math.floor(Math.random() * nNumOfFields);
  } while (0 !== board[nRandX][nRandY].length);
  
  return nRandX + "," + nRandY;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function moveRandom() {
  do {
    var nRandX = Math.floor(Math.random() * nNumOfFields);
    var nRandY = Math.floor(Math.random() * nNumOfFields);
    var neighbours = checkNeighbourhood(nRandX, nRandY);

    if (neighbours.length > 0) {
      var choose = Math.floor(Math.random() * neighbours.length);
      var tower = board[(neighbours[choose])[0]][(neighbours[choose])[1]];
      return (neighbours[choose])[0] + "," + (neighbours[choose])[1] + "|" +
          nRandX + "," + nRandY + "|" + tower.length;
    }
  } while (true);
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
