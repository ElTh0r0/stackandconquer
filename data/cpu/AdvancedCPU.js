/**
 * \file AdvancedCPU.js
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
 * Advanced CPU opponent.
 *
 * Following function can be used to access data from game:
 *   game.getID();
 *   game.getNumOfPlayers();
 *   game.getHeightToWin();
 *   game.getTowersNeededToWin();
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
    getMoveString(callCPU(game._jsboard, game._jsmoves, game._nDirection))
);
*/
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function initCPU() {
  game.log("Loading CPU script 'AdvancedCPU' with player ID " + game.getID());

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

  // "Scores" to be used finding the best move
  SET_NO_OWN_NEIGHBOUR = 1; // Place a stone on a field which has foreign neighbour(s)
  MOVE_NO_OWN_TOWER = 2; // Move a foreign tower
  SET_NO_NEIGHBOUR = 3; // Place a stone on a field without any neighbour
  SET_OWN_NEIGHBOUR = 5; // Place a stone on a field with own neighbour
  MOVE_TOWER_TOP2 = 10; // Build a tower of 2 with own top
  MOVE_TOWER_TOP3 = 20; // Build a tower of 3 with own top
  MOVE_TOWER_TOP4 = 50; // Build a tower of 4 with own top
  COULD_WIN_NEXT_ROUND_OPP_NEIGHBOUR = 90; // Possibility to win next round, but has foreign neighbour(s)
  COULD_WIN_NEXT_ROUND = 100; // Possibility to win next round and no foreign neighbour(s)

  DESTROY_OPP_TOWER3 = 30; // Destroy tower of 3 with foreign top, remaining tower has foreign top
  DESTROY_OPP_TOWER3_NEWTOP = 40; // Destroy tower of 3 with foreign top, remaining tower has own top
  DESTROY_OPP_TOWER4 = 150; // Destroy tower of 4 with foreign top
  DESTROY_OPP_TOWER4_NEWTOP = 200; // Destroy tower of 4 with foreign top, remaining tower has own top
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function callCPU(jsonBoard, jsonMoves, nDirection) {
  var board = JSON.parse(jsonBoard);
  // game.log("BOARD: " + jsonBoard);
  var legalMoves = JSON.parse(jsonMoves);
  // game.log("LEGAL MOVES: " + jsonMoves);
  shuffleArray(legalMoves);

  if (1 === legalMoves.length) {
    // Only one move possible, skip calculation
    game.log("CPU has only one possible move left");
    return legalMoves[0];
  }

  game.log("Possible moves: " + legalMoves.length);
  if (0 !== legalMoves.length) {
    return chooseMove(board, legalMoves);
  }

  // This line never should be reached!
  game.log("ERROR: No legal moves passed to script?!");
  return "";
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function shuffleArray(array) {
  for (var i = array.length - 1; i > 0; i--) {
    var j = Math.floor(Math.random() * (i + 1));
    var temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function chooseMove(currBoard, legalMoves) {
  var nNumOfPlayers = game.getNumOfPlayers();
  var nHeightTowerWin = game.getHeightToWin();
  var nScore = -9999;
  var bestmove = [-1, -1, -1];

  var cpuMoveToWin = canWin(currBoard, MY_ID, nHeightTowerWin);
  if (0 !== cpuMoveToWin.length) {
    // CPU can win
    if (isLegalMove(cpuMoveToWin[0], legalMoves)) {
      game.log("CPU can win!");
      return cpuMoveToWin[0];
    }
  }

  // Check if opponent could win on current board and try to prevent
  var prevWin = checkOpponentWinAndPrevent(
    currBoard,
    legalMoves,
    nHeightTowerWin
  );
  if (0 !== prevWin.length) {
    return prevWin;
  }

  // Store current winning moves to be compared with new board winning moves later
  var canWinMoves = [];
  for (var pID = 1; pID <= nNumOfPlayers; pID++) {
    var tempWinMoves = canWin(currBoard, pID, nHeightTowerWin);
    canWinMoves.push(tempWinMoves);
  }

  // Check what could happen after executing one of the legal moves
  skipMove: for (var i = 0; i < legalMoves.length; i++) {
    var newBoard = makePseudoMove(currBoard, legalMoves[i], MY_ID);
    game.log("Check PseudoMove " + getMoveString(legalMoves[i]));

    // Check if any opponent could win in *next* round if move will be executed
    for (var playerID = 1; playerID <= nNumOfPlayers; playerID++) {
      if (playerID !== MY_ID) {
        var movesToWin = canWin(newBoard, playerID, nHeightTowerWin);

        if (
          0 !== movesToWin.length && // Opponent could win in *next* round
          JSON.stringify(canWinMoves[playerID - 1]) !==
            JSON.stringify(movesToWin)
        ) {
          // New winning moves found (not already available before on old board)
          game.log(
            "Opponent #" +
              playerID +
              " could win after excuting move " +
              getMoveString(legalMoves[i]) +
              " - skipping this move!"
          );
          continue skipMove; // Skip move, since it could let to opponent win
        }
      }
    }

    // -------------------------------------------------
    // All "bad" moves have been sorted out above before

    // Check if CPU could win in *next* round
    var cpuToWin = canWin(newBoard, MY_ID, nHeightTowerWin);
    if (0 !== cpuToWin.length) {
      if (COULD_WIN_NEXT_ROUND > nScore) {
        nScore = COULD_WIN_NEXT_ROUND;
        bestmove = legalMoves[i].slice();
        game.log(
          "CPU" +
            MY_ID +
            " COULD_WIN_NEXT_ROUND " +
            getMoveString(legalMoves[i]) +
            " - score: " +
            nScore
        );
      }
    }

    if (-1 === legalMoves[i][0]) {
      // Set stone on empty field
      var newNeighbours = checkNeighbourhood(newBoard, legalMoves[i][2]);

      if (0 === newNeighbours.length) {
        // No neighbours at all
        if (SET_NO_NEIGHBOUR > nScore) {
          nScore = SET_NO_NEIGHBOUR;
          bestmove = legalMoves[i].slice();
          game.log(
            "SET_NO_NEIGHBOUR " +
              getMoveString(legalMoves[i]) +
              " - score: " +
              nScore
          );
        }
      } else {
        // Neighbour(s)
        var bFoundOppNeighbour = false;
        for (var point = 0; point < newNeighbours.length; point++) {
          // Check all neighbours
          var neighbourTower = newBoard[newNeighbours[point]];

          if (
            MY_ID !== parseInt(neighbourTower[neighbourTower.length - 1], 10)
          ) {
            // Neighbour = Opp
            bFoundOppNeighbour = true;

            if (
              COULD_WIN_NEXT_ROUND === nScore &&
              JSON.stringify(legalMoves[i]) === JSON.stringify(bestmove)
            ) {
              nScore = COULD_WIN_NEXT_ROUND_OPP_NEIGHBOUR;
              game.log("Reducing score! Found opponent neighbour.");
              game.log(
                "COULD_WIN_NEXT_ROUND_OPP_NEIGHBOUR " +
                  getMoveString(legalMoves[i]) +
                  " - score: " +
                  nScore
              );
            }
          }
        }

        if (bFoundOppNeighbour) {
          if (SET_NO_OWN_NEIGHBOUR > nScore) {
            nScore = SET_NO_OWN_NEIGHBOUR;
            bestmove = legalMoves[i].slice();
            game.log(
              "SET_NO_OWN_NEIGHBOUR " +
                getMoveString(legalMoves[i]) +
                " - score: " +
                nScore
            );
          }
        } else {
          if (SET_OWN_NEIGHBOUR > nScore) {
            nScore = SET_OWN_NEIGHBOUR;
            bestmove = legalMoves[i].slice();
            game.log(
              "SET_OWN_NEIGHBOUR " +
                getMoveString(legalMoves[i]) +
                " - score: " +
                nScore
            );
          }
        }
      }
    } else {
      // Move tower
      var sFrom = currBoard[legalMoves[i][0]];
      var nNum = legalMoves[i][1];
      var sTo = currBoard[legalMoves[i][2]];

      // Move tower and own color is on top of new tower
      if (MY_ID === parseInt(sFrom[sFrom.length - 1], 10)) {
        if (2 === nNum + sTo.length) {
          // Build stack of 2 stones
          if (MOVE_TOWER_TOP2 > nScore) {
            nScore = MOVE_TOWER_TOP2;
            bestmove = legalMoves[i].slice();
            game.log(
              "MOVE_TOWER_TOP2 " +
                getMoveString(legalMoves[i]) +
                " - score: " +
                nScore
            );
          }
        } else if (3 === nNum + sTo.length) {
          // Build stack of 3 stones
          if (MOVE_TOWER_TOP3 > nScore) {
            nScore = MOVE_TOWER_TOP3;
            bestmove = legalMoves[i].slice();
            game.log(
              "MOVE_TOWER_TOP3 " +
                getMoveString(legalMoves[i]) +
                " - score: " +
                nScore
            );
          }
        } else if (4 === nNum + sTo.length) {
          // Build stack of 4 stones
          if (MOVE_TOWER_TOP4 > nScore) {
            nScore = MOVE_TOWER_TOP4;
            bestmove = legalMoves[i].slice();
            game.log(
              "MOVE_TOWER_TOP4 " +
                getMoveString(legalMoves[i]) +
                " - score: " +
                nScore
            );
          }
        }
      }

      // Move tower and opponent color is on top of new tower
      if (MY_ID !== parseInt(sFrom[sFrom.length - 1], 10)) {
        var sNewFrom = sFrom.substring(sFrom.length - nNum, sFrom.length);

        if (4 === sFrom.length && nNum + sTo.length < 4) {
          // Destroy opponent tower with height 4 and new tower is not height 4 as well!
          if ("" !== sNewFrom) {
            if (MY_ID !== parseInt(sNewFrom[sNewFrom.length - 1], 10)) {
              // On from remains opponent stone on top
              if (DESTROY_OPP_TOWER4 > nScore) {
                nScore = DESTROY_OPP_TOWER4;
                bestmove = legalMoves[i].slice();
                game.log(
                  "DESTROY_OPP_TOWER4 " +
                    getMoveString(legalMoves[i]) +
                    " - score: " +
                    nScore
                );
              }
            } else {
              // On from remains own stone on top
              if (DESTROY_OPP_TOWER4_NEWTOP > nScore) {
                nScore = DESTROY_OPP_TOWER4_NEWTOP;
                bestmove = legalMoves[i].slice();
                game.log(
                  "DESTROY_OPP_TOWER4_NEWTOP " +
                    getMoveString(legalMoves[i]) +
                    " - score: " +
                    nScore
                );
              }
            }
          }
        } else if (3 === sFrom.length && nNum + sTo.length < 3) {
          // Destroy opponent tower with height 3 and new tower is not height 3 or heigher!
          if ("" !== sNewFrom) {
            if (MY_ID !== parseInt(sNewFrom[sNewFrom.length - 1], 10)) {
              // Opponent stone remains on top
              if (DESTROY_OPP_TOWER3 > nScore) {
                nScore = DESTROY_OPP_TOWER3;
                bestmove = legalMoves[i].slice();
                game.log(
                  "DESTROY_OPP_TOWER3 " +
                    getMoveString(legalMoves[i]) +
                    " - score: " +
                    nScore
                );
              }
            } else {
              // Own stone remains on top
              if (DESTROY_OPP_TOWER3_NEWTOP > nScore) {
                nScore = DESTROY_OPP_TOWER3_NEWTOP;
                bestmove = legalMoves[i].slice();
                game.log(
                  "DESTROY_OPP_TOWER3_NEWTOP " +
                    getMoveString(legalMoves[i]) +
                    " - score: " +
                    nScore
                );
              }
            }
          }
        } else {
          // Any other opponent tower move
          if (MOVE_NO_OWN_TOWER > nScore) {
            nScore = MOVE_NO_OWN_TOWER;
            bestmove = legalMoves[i].slice();
            game.log(
              "MOVE_NO_OWN_TOWER " +
                getMoveString(legalMoves[i]) +
                " - score: " +
                nScore
            );
          }
        }
      }
    }
  }

  // No "best" move found, choose random
  if (-1 === bestmove[0] && -1 === bestmove[1] && -1 === bestmove[2]) {
    var nRand = Math.floor(Math.random() * legalMoves.length);
    bestmove = legalMoves[nRand].slice();
    game.log(
      "No best move found, choosing randome move: " + getMoveString(bestmove)
    );
  } else {
    game.log("Best score: " + nScore + " - move: " + getMoveString(bestmove));
  }

  return bestmove;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function checkOpponentWinAndPrevent(currBoard, legalMoves, nHeightTowerWin) {
  var prevWin = [];

  skipMe: for (
    var opponentID = 1;
    opponentID <= game.getNumOfPlayers();
    opponentID++
  ) {
    if (opponentID === MY_ID) {
      // Skip loop for myself (was already checked before this function was called)
      continue skipMe;
    }

    var oppWinningMoves = canWin(currBoard, opponentID, nHeightTowerWin);
    if (0 !== oppWinningMoves.length) {
      for (var k = 0; k < oppWinningMoves.length; k++) {
        game.log(
          "Opponent #" +
            opponentID +
            " could win: " +
            getMoveString(oppWinningMoves[k])
        );
        prevWin = preventWin(
          currBoard,
          oppWinningMoves[k],
          legalMoves,
          nHeightTowerWin
        );
        if (0 !== prevWin.length) {
          game.log("Found preventive move");
          return prevWin;
        }
      }

      game.log("No move found to prevent opponent to win!!");
    }
  }
  return prevWin;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function preventWin(currBoard, moveToWin, legalMoves, nHeightTowerWin) {
  var nNumOfPlayers = game.getNumOfPlayers();

  var prevWin = [];
  var optimal = [];
  var pointFrom = moveToWin[0];
  var pointTo = moveToWin[2];

  /* Example for board 5x5 and padding 5
   * -16 -15 -14
   * -1   X    1
   * 14  15   16
   */
  var fieldsBetween;
  var direction = pointTo - pointFrom;
  var sign = direction >= 0 ? 1 : -1;

  // Just checking half of arry, since other half is just inverted -/+
  for (var dir = 0; dir < 4; dir++) {
    if (0 === direction % DIRS[dir]) {
      fieldsBetween = Math.abs(direction / DIRS[dir]) - 1;
      direction = sign * Math.abs(DIRS[dir]);
      break;
    }
  }

  if (fieldsBetween > 0) {
    // Check for blocking tower in between
    skipTry: for (var between = 1; between <= fieldsBetween; between++) {
      // ToDo: Evaluate neighbourhood when placing one stone
      var move = [];
      move.push(-1);
      move.push(1);
      move.push(pointTo - direction * between);

      if (isLegalMove(move, legalMoves)) {
        for (var playerID = 1; playerID <= nNumOfPlayers; playerID++) {
          if (playerID !== MY_ID) {
            var newBoard = makePseudoMove(currBoard, move, MY_ID);
            var oppCouldWin =
              canWin(newBoard, playerID, nHeightTowerWin).length > 0;
            if (oppCouldWin) {
              continue skipTry;
            }
          }
        }
        prevWin.push(move.slice());
        game.log(
          "Prevent opponent to win by blocking route: " + getMoveString(move)
        );
      }
    }

    // Search moves to destroy source or destination
    // Loop through legalMoves where source or destination is used in move.
    proceedLoop: for (var k = 0; k < legalMoves.length; k++) {
      if (
        pointFrom === legalMoves[k][0] ||
        pointFrom === legalMoves[k][2] ||
        pointTo === legalMoves[k][0] ||
        pointTo === legalMoves[k][2]
      ) {
        for (var playerID2 = 1; playerID2 <= nNumOfPlayers; playerID2++) {
          if (playerID2 !== MY_ID) {
            var newBoard2 = makePseudoMove(currBoard, legalMoves[k], MY_ID);
            var oppCouldWin2 =
              canWin(newBoard2, playerID2, nHeightTowerWin).length > 0;
            if (oppCouldWin2) {
              continue proceedLoop;
            }
          }
        }
        game.log(
          "Prevent opponent to win by destroying source/destination tower: " +
            getMoveString(legalMoves[k])
        );
        prevWin.push(legalMoves[k].slice());
      }
    }
  } else {
    // Source tower is next to destination, tower of height 4 has to be destroyed
    var sFrom = currBoard[pointFrom];
    // Remove from source max current height - 2 (= two stone remain)
    // otherwise opp could win in one of next rounds again
    skipTry2: for (var stones = 1; stones <= sFrom.length - 2; stones++) {
      var move2 = [];
      move2.push(pointFrom);
      move2.push(stones);
      move2.push(pointTo);

      for (var playerID3 = 1; playerID3 <= nNumOfPlayers; playerID3++) {
        if (playerID3 !== MY_ID) {
          var newBoard3 = makePseudoMove(currBoard, move2, MY_ID);
          var oppCouldWin3 =
            canWin(newBoard3, playerID3, nHeightTowerWin).length > 0;
          if (oppCouldWin3) {
            continue skipTry2;
          }
        }
      }

      if (isLegalMove(move2, legalMoves)) {
        game.log(
          "Prevent opponent to win by destroying destination: " +
            getMoveString(move2)
        );
        prevWin.push(move2.slice());

        var pseudo = makePseudoMove(currBoard, move2, MY_ID);
        if (canWin(pseudo, MY_ID, nHeightTowerWin).length > 0) {
          game.log(
            "Prevent opponent to win and possibility for CPU to win in next round: " +
              getMoveString(move2)
          );
          return move2;
        }

        // Check if own color could remain on top of source after moving stones
        if (0 === optimal.length) {
          if (
            MY_ID ===
            parseInt(
              sFrom.substring(sFrom.length - 1 - stones, sFrom.length - stones),
              10
            )
          ) {
            game.log(
              "Prevent opponent to win and own stone on top of source tower: " +
                getMoveString(move2)
            );
            optimal = move2.slice();
          }
        }
      }
    }

    // Check if destination could be destroyed
    proceedLoop2: for (var j = 0; j < legalMoves.length; j++) {
      if (pointTo === legalMoves[j][0]) {
        for (var playerID4 = 1; playerID4 <= nNumOfPlayers; playerID4++) {
          if (playerID4 !== MY_ID) {
            var newBoard4 = makePseudoMove(currBoard, legalMoves[j], MY_ID);
            var oppCouldWin4 =
              canWin(newBoard4, playerID4, nHeightTowerWin).length > 0;
            if (oppCouldWin4) {
              continue proceedLoop2;
            }
          }
        }
        game.log(
          "Prevent opponent to win by removing destination tower: " +
            getMoveString(legalMoves[j])
        );
        prevWin.push(legalMoves[j].slice());
      }
    }
  }

  if (optimal.length > 0) {
    return optimal;
  }

  if (prevWin.length > 0) {
    var nRand = Math.floor(Math.random() * prevWin.length);
    return prevWin[nRand];
  } else {
    return prevWin;
  }
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

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function makePseudoMove(currBoard, move, nPlayerID) {
  // Create copy of current board and return new board after executing move
  // move[0]: From (-1 --> set stone at "To")
  // move[1]: Number of stones
  // move[2]: To
  var newBoard = currBoard.slice();

  if (-1 === move[0]) {
    // Set stone
    newBoard[move[2]] = nPlayerID.toString();
  } else {
    // Move tower
    var sFieldFrom = newBoard[move[0]];
    var sMovedStones = sFieldFrom.substring(
      sFieldFrom.length - move[1],
      sFieldFrom.length
    );
    // Remove stones from source field
    newBoard[move[0]] = sFieldFrom.slice(0, sFieldFrom.length - move[1]);
    // Add stones to destination field
    newBoard[move[2]] = newBoard[move[2]] + sMovedStones;
  }
  return newBoard;
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

function getFieldFromIndex(nIndex) {
  var nTop =
    game.getHeightToWin() *
    (2 * game.getHeightToWin() + game.getBoardDimensionX());
  var nFirst = nTop + game.getHeightToWin();
  var nLeftRight =
    2 *
    game.getHeightToWin() *
    (Math.floor(
      nIndex / (game.getBoardDimensionX() + 2 * game.getHeightToWin())
    ) -
      game.getHeightToWin());
  return nIndex - nFirst - nLeftRight;
}

function getCoordinateFromField(nField) {
  var x = nField % game.getBoardDimensionX();
  var y = Math.floor(nField / game.getBoardDimensionX());
  return [x, y];
}

function getStringCoordinateFromIndex(nIndex) {
  var coord = getCoordinateFromField(getFieldFromIndex(nIndex));
  return String.fromCharCode(coord[0] + 65) + (coord[1] + 1);
}

function getMoveString(move) {
  if (-1 === move[0]) {
    return "-1:" + move[1] + "-" + getStringCoordinateFromIndex(move[2]);
  }
  return (
    getStringCoordinateFromIndex(move[0]) +
    ":" +
    move[1] +
    "-" +
    getStringCoordinateFromIndex(move[2])
  );
}
