/**
 * \file DummyCPU.js
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
 * Dummy CPU opponent.
 *
 * Variables provided externally from game:
 * jsboard
 * jsmoves
 * nID (1 or 2 = player 1 / player 2)
 * nBoardDimensionsX
 * nBoardDimensionsY
 * nHeightTowerWin
 * sOut
 * sPad
 */

cpu.log("Loading CPU script DummyCPU...");

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function callCPU() {
  board = JSON.parse(jsboard);  // Global
  legalMoves = JSON.parse(jsmoves);  // Global
  /*
   * Moving directions factor
   * E.g. 5x5 board, max tower height 5 (padding):
   * -16 -15 -14
   * -1   X    1
   * 14  15   16
   */
  DIRS = [];
  DIRS.push(-(2 * nHeightTowerWin + nBoardDimensionsX + 1));  // -16
  DIRS.push(-(2 * nHeightTowerWin + nBoardDimensionsX));      // -15
  DIRS.push(-(2 * nHeightTowerWin + nBoardDimensionsX - 1));  // -14
  DIRS.push(-1);  // -1
  DIRS.push(1);   //  1
  DIRS.push(-DIRS[2]);  // 14
  DIRS.push(-DIRS[1]);  // 15
  DIRS.push(-DIRS[0]);  // 16

  //cpu.log("board[80].length: " + board[80].length);
  //cpu.log("board[81].length: " + board[81].length);
  //cpu.log("board[81][0]: " + board[81][0]);

  var MoveToWin = canWin(nID);
  if (0 !== MoveToWin.length) {  // CPU can win
    return MoveToWin[0];
  }

  // Check if opponent can win
  if (2 === nID) {
    MoveToWin = canWin(1);
  } else {
    MoveToWin = canWin(2);
  }
  if (0 !== MoveToWin.length) {
    // TODO(x): Rewrite preventWin() with new legal moves list
    // var PreventWin = preventWin(MoveToWin[0], nPossibleMove);
    // if (3 === PreventWin.length) {
    //  return PreventWin;
    // }
  }

  cpu.log("Possible moves: " + legalMoves.length);
  if (0 !== legalMoves.length) {
    //cpu.log("Possible moves #1: " + legalMoves[0]);
    //cpu.log("Possible moves #1.1: " + legalMoves[0][0]);
    //cpu.log("Possible moves #1.2: " + legalMoves[0][1]);
    //cpu.log("Possible moves #1.3: " + legalMoves[0][2]);

    // Make random move
    var nRand = Math.floor(Math.random() * legalMoves.length);
    return legalMoves[nRand];
  }

  // This line never should be reached!
  cpu.log("ERROR: No legal moves passed to script?!");
  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function checkNeighbourhood(nIndex) {
  var neighbours = [];
  var nMoves = board[nIndex].length;

  if (0 === nMoves) {
    return neighbours;
  }

  var sField = "";
  for (var dir = 0; dir < DIRS.length; dir++) {
    for (var range = 1; range <= nMoves; range++) {
      sField = board[nIndex + DIRS[dir]*range];
      if (0 !== sField.length && range < nMoves) {  // Route blocked
        break;
      }
      if (!isNaN(parseInt(sField, 10)) && range === nMoves) {
        neighbours.push(nIndex + DIRS[dir]*range);
      }
    }
  }

  return neighbours;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function canWin(nPlayerID) {
  var ret = [];
  for (var nIndex = 0; nIndex < board.length; nIndex++) {
    if (0 !== board[nIndex].length &&
        sOut !== board[nIndex] &&
        sPad !== board[nIndex]) {
      var neighbours = checkNeighbourhood(nIndex);
      for (var point = 0; point < neighbours.length; point++) {
        var tower = board[(neighbours[point])];
        if ((board[nIndex].length + tower.length >= nHeightTowerWin) &&
            nPlayerID === parseInt(tower[tower.length - 1], 10)) {  // Top = own
          var move = [];  // From, num of stones, to
          move.push(neighbours[point]);
          move.push(tower.length);
          move.push(nIndex);
          ret.push(move);  // Generate list of all opponent winning moves

          if (nPlayerID === nID) {  // Return first found move for CPU to win
            return ret;
          }
        }
      }
    }
  }
  return ret;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function preventWin(moveToWin, nPossibleMove) {
  var move = [];
  var pointFrom = moveToWin[0];
  var nNumber = moveToWin[1];
  var pointTo = moveToWin[2];

  // Check if a blocking towers in between can be placed
  var route = pointTo - pointFrom;
  for (var dir = 0; dir < DIRS.length; dir++) {
    if (1 === Math.abs(DIRS[dir])) {  // +1 / -1
      if (Math.abs(route) < (nBoardDimensionsX-2) &&
          (route > 0 && DIRS[dir] < 0 ||
           route < 0 && DIRS[dir] > 0)) {
        move.push(-1);
        move.push(1);
        move.push(pointTo + DIRS[dir]);
        return move;
      }
    } else {
      if (0 === route % DIRS[dir] &&       // There is a route between points
          (route > 0 && DIRS[dir] < 0 ||   // If route pos., dir has to be neg.
           route < 0 && DIRS[dir] > 0)) {  // If route neg., dir has to be pos.
        var moves = route / DIRS[dir];
        // There is more than one filed in between
        if (Math.abs(moves) > 1 && (1 === nPossibleMove || 3 === nPossibleMove)) {
          move.push(-1);
          move.push(1);
          move.push(pointTo + DIRS[dir]);
          return move
        }
        // if (nPossibleMove >= 2) {
        // TODO(x): Try to move tower to prevent win
        // }
      }
    }
  }

  return move;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function setRandom() {
  // Seed random?
  var move = [];
  var nRand;
  do {
    nRand = Math.floor(Math.random() * board.length);
  } while (0 !== board[nRand].length);

  move.push(-1);
  move.push(1);
  move.push(nRand);
  return move
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function moveRandom(oppWinning) {
  var nCnt = 0;
  do {
    var bBreak = false;
    var nRandTo = Math.floor(Math.random() * board.length);
    if (sOut !== board[nRandTo] && sPad !== board[nRandTo]) {
      var neighbours = checkNeighbourhood(nRandTo);

      if (neighbours.length > 0) {
        var choose = Math.floor(Math.random() * neighbours.length);
        // Tower from which stones are moved:
        var nMaxStones = board[(neighbours[choose])].length;
        var move = [];  // From, num of stones, to
        move.push(neighbours[choose]);
        move.push(Math.floor(Math.random() * nMaxStones) + 1);
        move.push(nRandTo);

        // Dumb workaround for trying to find another
        // move which doesn't let opponent win.
        nCnt += 1;
        if (nCnt < 10) {
          for (var i = 0; i < oppWinning.length; i++) {
            if (oppWinning[i][0] === move[0] &&
                oppWinning[i][1] === move[1] &&
                oppWinning[i][2] === move[2]) {
              bBreak = true;
              break;
            }
          }
          if (true === bBreak) {
            // cpu.log("Trying to find another move...");
            continue;
          }
        }

        return move;
      }
    }
  } while (true);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function findFreeFields() {
  for (var nIndex = 0; nIndex < board.length; nIndex++) {
    if (0 === board[nIndex].length) {
      return true;
    }
  }
  return false;
}
