/**
 * \file DummyCPU.js
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
 * Dummy CPU opponent.
 *
 * Following function can be used to access data from game:
 *   game.getID();
 *   game.getNumOfPlayers();
 *   game.getHeightToWin();
 *   game.getTowersToWin();
 *   game.getScores();
 *   game.getBoardDimensionX();
 *   game.getBoardDimensionY();
 *   game.getOutside();
 *   game.getPadding();
 *
 */

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// !!! Uncomment block for script debugging only !!!
/*
var game = {
  _nID: 2,
  _nNumOfPlayers: 2,
  _nHeightTowerWin: 5,
  _nBoardDimensionX: 5,
  _nBoardDimensionY: 5,
  _sOut: "#",
  _sPad: "-",

  log: function (sMessage) {
    console.log(sMessage);
  },
  getID: function () {
    return this._nID;
  },
  getNumOfPlayers: function () {
    return this._nNumOfPlayers;
  },
  getHeightToWin: function () {
    return this._nHeightTowerWin;
  },
  getBoardDimensionX: function () {
    return this._nBoardDimensionX;
  },
  getBoardDimensionY: function () {
    return this._nBoardDimensionY;
  },
  getOutside: function () {
    return this._sOut;
  },
  getPadding: function () {
    return this._sPad;
  },

  _jsboard:
    '["-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","1","","2","22","222","-","-","-","-","-","-","-","-","-","-","","","","","111","-","-","-","-","-","-","-","-","-","-","2","","122","","","-","-","-","-","-","-","-","-","-","-","","","","","","-","-","-","-","-","-","-","-","-","-","1111","1","","1","","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-","-"]',

  _jsmoves: JSON.stringify([
    [-1, 1, 81],
    [83, 1, 82],
    [83, 2, 82],
    [-1, 1, 95],
    [-1, 1, 96],
    [-1, 1, 97],
    [-1, 1, 98],
    [141, 1, 99],
    [-1, 1, 111],
    [-1, 1, 113],
    [-1, 1, 114],
    [-1, 1, 125],
    [-1, 1, 126],
    [-1, 1, 127],
    [-1, 1, 128],
    [-1, 1, 129],
    [140, 1, 141],
    [140, 2, 141],
    [140, 3, 141],
    [-1, 1, 142],
    [-1, 1, 144],
  ]),

  _nDirection: 1,
};

initCPU();
console.log(
  "CPU script returned: " +
    callCPU(game._jsboard, game._jsmoves, game._nDirection)
);
*/
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function initCPU() {
  game.log("Loading CPU script 'DummyCPU' with player ID " + game.getID());

  /* global MY_ID:writable, DIRS:writable */
  MY_ID = game.getID();
  /*
   * Moving directions factor
   * E.g. 5x5 board, max tower height 5 (padding):
   * -16 -15 -14
   * -1   X    1
   * 14  15   16
   */
  DIRS = [];
  DIRS.push(-(2 * game.getHeightToWin() + game.getBoardDimensionX() + 1)); // -16
  DIRS.push(-(2 * game.getHeightToWin() + game.getBoardDimensionX())); // -15
  DIRS.push(-(2 * game.getHeightToWin() + game.getBoardDimensionX() - 1)); // -14
  DIRS.push(-1); // -1
  DIRS.push(1); //  1
  DIRS.push(-DIRS[2]); // 14
  DIRS.push(-DIRS[1]); // 15
  DIRS.push(-DIRS[0]); // 16
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function callCPU(jsonBoard, jsonMoves, nDirection) {
  var board = JSON.parse(jsonBoard);
  // game.log("BOARD: " + jsonBoard);
  var legalMoves = JSON.parse(jsonMoves);
  // game.log("LEGAL MOVES: " + jsonMoves);

  var moveToWin = canWin(board, MY_ID, game.getHeightToWin());
  if (0 !== moveToWin.length) {
    // CPU can win
    game.log("CPU can win!");
    return moveToWin[0];
  }

  if (1 === legalMoves.length) {
    // Only one move possible, skip calculation
    game.log("CPU has only one possible move left");
    return legalMoves[0];
  }

  // Check if next opponent can win depending on playing direction
  var prevWin;
  if (nDirection > 0) {
    if (MY_ID === game.getNumOfPlayers()) {
      moveToWin = canWin(board, 1, game.getHeightToWin());
    } else {
      moveToWin = canWin(board, MY_ID + 1, game.getHeightToWin());
    }
    if (0 !== moveToWin.length) {
      prevWin = preventWin(board, moveToWin[0], legalMoves);
      if (3 === prevWin.length) {
        return prevWin;
      }
    }
  } else {
    if (MY_ID === 1) {
      moveToWin = canWin(board, game.getNumOfPlayers(), game.getHeightToWin());
    } else {
      moveToWin = canWin(board, MY_ID - 1, game.getHeightToWin());
    }
    if (0 !== moveToWin.length) {
      prevWin = preventWin(board, moveToWin[0], legalMoves);
      if (3 === prevWin.length) {
        return prevWin;
      }
    }
  }

  game.log("Possible moves: " + legalMoves.length);
  if (0 !== legalMoves.length) {
    // Make random move
    var nRand = Math.floor(Math.random() * legalMoves.length);
    return legalMoves[nRand];
  }

  // This line never should be reached!
  game.log("ERROR: No legal moves passed to script?!");
  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function checkNeighbourhood(currBoard, nIndex) {
  var neighbours = [];
  var nMoves = currBoard[nIndex].length;

  if (0 === nMoves) {
    return neighbours;
  }

  var sField = "";
  for (var dir = 0; dir < DIRS.length; dir++) {
    for (var range = 1; range <= nMoves; range++) {
      sField = currBoard[nIndex + DIRS[dir] * range];
      if (0 !== sField.length && range < nMoves) {
        // Route blocked
        break;
      }
      if (!isNaN(parseInt(sField, 10)) && range === nMoves) {
        neighbours.push(nIndex + DIRS[dir] * range);
      }
    }
  }

  return neighbours;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function canWin(currBoard, nPlayerID, nHeightTowerWin) {
  var sOut = game.getOutside();
  var sPad = game.getPadding();

  var ret = [];
  for (var nIndex = 0; nIndex < currBoard.length; nIndex++) {
    if (
      0 !== currBoard[nIndex].length &&
      sOut !== currBoard[nIndex] &&
      sPad !== currBoard[nIndex]
    ) {
      var neighbours = checkNeighbourhood(currBoard, nIndex);
      for (var point = 0; point < neighbours.length; point++) {
        var tower = currBoard[neighbours[point]];
        if (
          currBoard[nIndex].length + tower.length >= nHeightTowerWin &&
          nPlayerID === parseInt(tower[tower.length - 1], 10)
        ) {
          // Top = own
          var move = []; // From, num of stones, to
          move.push(neighbours[point]);
          move.push(tower.length);
          move.push(nIndex);
          ret.push(move); // Generate list of all opponent winning moves

          if (nPlayerID === MY_ID) {
            // Return first found move for CPU to win
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

function preventWin(currBoard, moveToWin, legalMoves) {
  var nBoardDimensionsX = game.getBoardDimensionX();
  //var prevWin = [];
  var move = [];
  var pointFrom = moveToWin[0];
  //var nNumber = moveToWin[1];
  var pointTo = moveToWin[2];

  // Check if a blocking towers in between can be placed
  var route = pointTo - pointFrom;
  for (var dir = 0; dir < DIRS.length; dir++) {
    if (1 === Math.abs(DIRS[dir])) {
      // +1 / -1
      if (
        Math.abs(route) < nBoardDimensionsX - 2 &&
        ((route > 0 && DIRS[dir] < 0) || (route < 0 && DIRS[dir] > 0))
      ) {
        //move = [];
        move.push(-1);
        move.push(1);
        move.push(pointTo + DIRS[dir]);

        if (isLegalMove(move, legalMoves)) {
          //prevWin.push(move);
          return move;
        } else {
          move.pop();
        }
      }
    } else {
      if (
        0 === route % DIRS[dir] && // There is a route between points
        ((route > 0 && DIRS[dir] < 0) || // If route pos., dir has to be neg.
          (route < 0 && DIRS[dir] > 0))
      ) {
        // If route neg., dir has to be pos.
        var moves = route / DIRS[dir];
        // There is more than one field in between
        if (Math.abs(moves) > 1) {
          //move = [];
          move.push(-1);
          move.push(1);
          move.push(pointTo + DIRS[dir]);

          if (isLegalMove(move, legalMoves)) {
            //prevWin.push(move);
            return move;
          } else {
            move.pop();
          }
        }

        // TODO(x): Try to move tower to prevent win
      }
    }
  }

  /*
  if (prevWin.length > 0) {
    var nRand = Math.floor(Math.random() * prevWin.length);
    return prevWin[nRand];
  } else {
    return prevWin;
  }
  */
  return move;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function isLegalMove(move, legalMoves) {
  var sMove = JSON.stringify(move);
  for (var i = 0; i < legalMoves.length; i++) {
    if (JSON.stringify(legalMoves[i]) === sMove) {
      return true;
    }
  }

  return false;
}
