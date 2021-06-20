/**
 * \file AdvancedCPU.js
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2021 Thorsten Roth
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
 *   game.getBoardDimension();
 *   game.getOutside();
 *   game.getPadding();
 */

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// !!! Uncomment block for script debugging only !!!
/*
class DebugCPU {
  #nID = 2;
  #nNumOfPlayers = 2;
  #nHeightTowerWin = 5;
  #nBoardDimensionsX = 5;
  #nBoardDimensionsY = 5;
  #sOut = "#";
  #sPad = "-";
  
  log(sMessage) {
    console.log(sMessage);
  }
  getID() {
    return this.#nID;
  }
  getNumOfPlayers() {
    return this.#nNumOfPlayers;
  }
  getHeightToWin() {
    return this.#nHeightTowerWin;
  }
  getBoardDimension() {
    return [this.#nBoardDimensionsX, this.#nBoardDimensionsY];
  }
  getOutside() {
    return this.#sOut;
  }
  getPadding() {
    return this.#sPad;
  }
}
game = new DebugCPU();

let jsboard = "[\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"\",\"1111\",\"11\",\"1\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"\",\"1\",\"\",\"\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"\",\"\",\"\",\"222\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"\",\"\",\"\",\"22\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"\",\"2\",\"\",\"\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\"]";
let jsmoves = JSON.stringify([[-1,1,80],[-1,1,81],[83,1,84],[83,2,84],[-1,1,95],[-1,1,96],[82,1,97],[82,2,97],[82,3,97],[83,1,97],[83,2,97],[-1,1,98],[-1,1,99],[-1,1,110],[-1,1,111],[-1,1,112],[-1,1,113],[-1,1,125],[-1,1,126],[-1,1,127],[-1,1,128],[97,1,129],[-1,1,140],[-1,1,141],[-1,1,143],[-1,1,144]]);
let nDirection = 1;

initCPU();
console.log('CPU script returned: ' + getMoveString(callCPU(jsboard, jsmoves, nDirection)));
*/
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function initCPU() {
  game.log("Loading CPU script 'AdvancedCPU' with player ID " + game.getID());

  // Global variables
  MY_ID = game.getID();
  /*
   * Moving directions factor
   * E.g. 5x5 board, max tower height 5 (padding):
   * -16 -15 -14
   * -1   X    1
   * 14  15   16
   */
  DIRS = [];
  DIRS.push(-(2 * game.getHeightToWin() + game.getBoardDimension()[0] + 1));  // -16
  DIRS.push(-(2 * game.getHeightToWin() + game.getBoardDimension()[0]));      // -15
  DIRS.push(-(2 * game.getHeightToWin() + game.getBoardDimension()[0] - 1));  // -14
  DIRS.push(-1);  // -1
  DIRS.push(1);   //  1
  DIRS.push(-DIRS[2]);  // 14
  DIRS.push(-DIRS[1]);  // 15
  DIRS.push(-DIRS[0]);  // 16
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function callCPU(jsonBoard, jsonMoves, nDirection) {
  let board = JSON.parse(jsonBoard);
  // game.log("BOARD: " + jsonBoard);
  let legalMoves = JSON.parse(jsonMoves);
  // game.log("LEGAL MOVES: " + jsonMoves);
  shuffleArray(legalMoves);

  if (1 === legalMoves.length) {  // Only one move possible, skip calculation
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
  for (let i = array.length - 1; i > 0; i--) {
    let j = Math.floor(Math.random() * (i + 1));
    let temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function chooseMove(currBoard, legalMoves) {
  const nNumOfPlayers = game.getNumOfPlayers();
  const nHeightTowerWin = game.getHeightToWin();
  
  const SET_NO_OWN_NEIGHBOUR = 1;
  const MOVE_NO_OWN_TOWER = 2;
  const SET_NO_NEIGHBOUR = 3;
  const SET_OWN_NEIGHBOUR = 5;
  const MOVE_TOWER_TOP2 = 10;
  const MOVE_TOWER_TOP3 = 20;
  const MOVE_TOWER_TOP4 = 50;
  const COULD_WIN_NEXT_ROUND_OPP_NEIGHBOUR = 90;
  const COULD_WIN_NEXT_ROUND = 100;

  const DESTROY_OPP_TOWER3 = 30;
  const DESTROY_OPP_TOWER3_NEWTOP = 40;
  const DESTROY_OPP_TOWER4 = 150;
  const DESTROY_OPP_TOWER4_NEWTOP = 200;

  let nScore = -9999;
  let bestmove = [-1, -1, -1];

  let cpuMoveToWin = canWin(currBoard, MY_ID, nHeightTowerWin);
  if (0 !== cpuMoveToWin.length) {  // CPU can win
    if (isLegalMove(cpuMoveToWin[0], legalMoves)) {
      game.log("CPU can win!");
      return cpuMoveToWin[0];
    }
  }

  // Check if opponent could win on current board
  skipMe: for (let opponentID = 1; opponentID <= nNumOfPlayers; opponentID++) {
    if (opponentID === MY_ID) {  // Skip loop for myself (was already checked in previsous loop)
      continue skipMe;
    }

    let oppWinningMoves = canWin(currBoard, opponentID, nHeightTowerWin);
    if (0 !== oppWinningMoves.length) {
      for (let k = 0; k < oppWinningMoves.length; k++) {
        game.log("Opponent #" + opponentID + " could win: " + getMoveString(oppWinningMoves[k]));
        let prevWin = preventWin(currBoard, oppWinningMoves[k], legalMoves, nHeightTowerWin);
        if (0 !== prevWin.length) {
          game.log("Found preventive move");
          return prevWin;
        }
      }

      game.log("No move found to prevent opponent to win!!");
    }
  }

  // Store current winning moves to be compared with new board winning moves later
  let canWinMoves = [];
  for (let pID = 1; pID <= nNumOfPlayers; pID++) {
    let tempWinMoves = canWin(currBoard, pID, nHeightTowerWin);
    canWinMoves.push(tempWinMoves);
  }

  // Check what could happen after executing one of the legal moves
  skipMove: for (let i = 0; i < legalMoves.length; i++) {
    let newBoard = makePseudoMove(currBoard, legalMoves[i], MY_ID);
    game.log("Check PseudoMove " + getMoveString(legalMoves[i]));

    // Check if any opponent could win in *next* round if move will be executed
    for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
      if (playerID !== MY_ID) {
        let movesToWin = canWin(newBoard, playerID, nHeightTowerWin);

        if (0 !== movesToWin.length &&  // Opponent could win in *next* round
          JSON.stringify(canWinMoves[playerID - 1]) !== JSON.stringify(movesToWin)) {  // New winning moves found (not already available before on old board)
          game.log("Opponent #" + playerID + " could win after excuting move " + getMoveString(legalMoves[i]) + " - skipping this move!");
          continue skipMove;  // Skip move, since it could let to opponent win
        }
      }
    }

    // -------------------------------------------------
    // All "bad" moves have been sorted out above before

    // Check if CPU could win in *next* round
    let cpuToWin = canWin(newBoard, MY_ID, nHeightTowerWin);
    if (0 !== cpuToWin.length) {
      if (COULD_WIN_NEXT_ROUND > nScore) {
        nScore = COULD_WIN_NEXT_ROUND;
        bestmove = legalMoves[i].slice();
        game.log("CPU" + MY_ID + " COULD_WIN_NEXT_ROUND " + getMoveString(legalMoves[i]) + " - score: " + nScore);
      }
    }

    if (-1 === legalMoves[i][0]) {  // Set stone on empty field
      let newNeighbours = checkNeighbourhood(newBoard, legalMoves[i][2]);

      if (0 === newNeighbours.length) {  // No neighbours at all
        if (SET_NO_NEIGHBOUR > nScore) {
          nScore = SET_NO_NEIGHBOUR;
          bestmove = legalMoves[i].slice();
          game.log("SET_NO_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
        }
      } else {  // Neighbour(s)
        let bFoundOppNeighbour = false;
        for (let point = 0; point < newNeighbours.length; point++) {  // Check all neighbours
          let neighbourTower = newBoard[(newNeighbours[point])];

          if (MY_ID !== parseInt(neighbourTower[neighbourTower.length - 1], 10)) {  // Neighbour = Opp
            bFoundOppNeighbour = true;

            if (COULD_WIN_NEXT_ROUND === nScore &&
              JSON.stringify(legalMoves[i]) === JSON.stringify(bestmove)) {
              nScore = COULD_WIN_NEXT_ROUND_OPP_NEIGHBOUR;
              game.log("Reducing score! Found opponent neighbour.");
              game.log("COULD_WIN_NEXT_ROUND_OPP_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
            }
          }
        }

        if (bFoundOppNeighbour) {
          if (SET_NO_OWN_NEIGHBOUR > nScore) {
            nScore = SET_NO_OWN_NEIGHBOUR;
            bestmove = legalMoves[i].slice();
            game.log("SET_NO_OWN_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        } else {
          if (SET_OWN_NEIGHBOUR > nScore) {
            nScore = SET_OWN_NEIGHBOUR;
            bestmove = legalMoves[i].slice();
            game.log("SET_OWN_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        }
      }
    } else {  // Move tower
      let sFrom = currBoard[legalMoves[i][0]];
      let nNum = legalMoves[i][1];
      let sTo = currBoard[legalMoves[i][2]];

      // Move tower and own color is on top of new tower
      if (MY_ID === parseInt(sFrom[sFrom.length - 1], 10)) {
        if (2 === (nNum + sTo.length)) {  // Build stack of 2 stone
          if (MOVE_TOWER_TOP2 > nScore) {
            nScore = MOVE_TOWER_TOP2;
            bestmove = legalMoves[i].slice();
            game.log("MOVE_TOWER_TOP2 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        } else if (3 === (nNum + sTo.length)) {  // Build stack of 3 stone
          if (MOVE_TOWER_TOP3 > nScore) {
            nScore = MOVE_TOWER_TOP3;
            bestmove = legalMoves[i].slice();
            game.log("MOVE_TOWER_TOP3 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        } else if (4 === (nNum + sTo.length)) {  // Build stack of 4 stone
          if (MOVE_TOWER_TOP4 > nScore) {
            nScore = MOVE_TOWER_TOP4;
            bestmove = legalMoves[i].slice();
            game.log("MOVE_TOWER_TOP4 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        }
      }

      // Move tower and opponent color is on top of new tower
      if (MY_ID !== parseInt(sFrom[sFrom.length - 1], 10)) {
        let sNewFrom = sFrom.substring(sFrom.length - nNum, sFrom.length);

        if (4 === sFrom.length &&     // Destroy opponent tower with height 4
          (nNum + sTo.length) < 4) {  // and new tower is not height 4 as well!
          if ("" !== sNewFrom) {
            if (MY_ID !== parseInt(sNewFrom[sNewFrom.length - 1], 10)) {  // On from remains opponont stone on top
              if (DESTROY_OPP_TOWER4 > nScore) {
                nScore = DESTROY_OPP_TOWER4;
                bestmove = legalMoves[i].slice();
                game.log("DESTROY_OPP_TOWER4 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            } else {  // On from remains own stone on top
              if (DESTROY_OPP_TOWER4_NEWTOP > nScore) {
                nScore = DESTROY_OPP_TOWER4_NEWTOP;
                bestmove = legalMoves[i].slice();
                game.log("DESTROY_OPP_TOWER4_NEWTOP " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            }
          }
        } else if (3 === sFrom.length &&  // Destroy opponent tower with height 3
          (nNum + sTo.length) < 3) {      // and new tower is not height 3 or heigher!
          if ("" !== sNewFrom) {
            if (MY_ID !== parseInt(sNewFrom[sNewFrom.length - 1], 10)) {  // On from remains opponont stone on top
              if (DESTROY_OPP_TOWER3 > nScore) {
                nScore = DESTROY_OPP_TOWER3;
                bestmove = legalMoves[i].slice();
                game.log("DESTROY_OPP_TOWER3 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            } else {  // On from remains own stone on top
              if (DESTROY_OPP_TOWER3_NEWTOP > nScore) {
                nScore = DESTROY_OPP_TOWER3_NEWTOP;
                bestmove = legalMoves[i].slice();
                game.log("DESTROY_OPP_TOWER3_NEWTOP " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            }
          }
        } else {  // Any other opponent tower move
          if (MOVE_NO_OWN_TOWER > nScore) {
            nScore = MOVE_NO_OWN_TOWER;
            bestmove = legalMoves[i].slice();
            game.log("MOVE_NO_OWN_TOWER " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        }
      }
    }
  }

  // No "best" move found, choose random
  if (-1 === bestmove[0] &&
    -1 === bestmove[1] &&
    -1 === bestmove[2]) {
    let nRand = Math.floor(Math.random() * legalMoves.length);
    bestmove = legalMoves[nRand].slice();
    game.log("No best move found, choosing randome move: " + getMoveString(bestmove));
  } else {
    game.log("Best score: " + nScore + " - move: " + getMoveString(bestmove));
  }

  return bestmove;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function preventWin(currBoard, moveToWin, legalMoves, nHeightTowerWin) {
  const nNumOfPlayers = game.getNumOfPlayers();
  const nBoardDimensionsX = game.getBoardDimension()[0];

  let prevWin = [];
  let optimal = [];
  let pointFrom = moveToWin[0];
  let pointTo = moveToWin[2];

  /* Example for board 5x5 and padding 5
  * -16 -15 -14
  * -1   X    1
  * 14  15   16
  */
  let fieldsBetween;
  let direction = pointTo - pointFrom;
  let sign = Math.sign(direction);
  for (let dir = 0; dir < 4; dir++) {  // Just checking half of arry, since other half is just inverted -/+
    if (0 === direction % DIRS[dir]) {
      fieldsBetween = Math.abs(direction / DIRS[dir]) - 1;
      direction = sign * Math.abs(DIRS[dir]);
      break;
    }
  }

  if (fieldsBetween > 0) {
    // Check for blocking tower in between
    skipTry: for (between = 1; between <= fieldsBetween; between++) {
      // ToDo: Evaluate neighbourhood when placing one stone
      let move = [];
      move.push(-1);
      move.push(1);
      move.push(pointTo - (direction * between));

      if (isLegalMove(move, legalMoves)) {
        for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
          if (playerID !== MY_ID) {
            let newBoard = makePseudoMove(currBoard, move, MY_ID);
            let oppCouldWin = (canWin(newBoard, playerID, nHeightTowerWin).length > 0);
            if (oppCouldWin) {
              continue skipTry;
            }
          }
        }
        prevWin.push(move.slice());
        game.log("Prevent opponent to win by blocking route: " + getMoveString(move));
      }
    }
    
    // Search moves to destroy source or destination
    // Loop through legalMoves where source or destination is used in move.
    proceedLoop: for (let k = 0; k < legalMoves.length; k++) {
      if (pointFrom === legalMoves[k][0] ||
        pointFrom === legalMoves[k][2] ||
        pointTo === legalMoves[k][0] ||
        pointTo === legalMoves[k][2]) {
        for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
          if (playerID !== MY_ID) {
            let newBoard2 = makePseudoMove(currBoard, legalMoves[k], MY_ID);
            let oppCouldWin = (canWin(newBoard2, playerID, nHeightTowerWin).length > 0);
            if (oppCouldWin) {
              continue proceedLoop;
            }
          }
        }
        game.log("Prevent opponent to win by destroying source/destination tower: " + getMoveString(legalMoves[k]));
        prevWin.push(legalMoves[k].slice());
      }
    }

  } else {  // Source tower is next to destination, tower of height 4 has to be destroyed
    let sFrom = currBoard[pointFrom];
    // Remove from source max current height - 2 (= two stone remain)
    // otherwise opp could win in one of next rounds again
    skipTry2: for (stones = 1; stones <= sFrom.length - 2; stones++) {
      let move = [];
      move.push(pointFrom);
      move.push(stones);
      move.push(pointTo);
      
      for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
        if (playerID !== MY_ID) {
          let newBoard = makePseudoMove(currBoard, move, MY_ID);
          let oppCouldWin = (canWin(newBoard, playerID, nHeightTowerWin).length > 0);
          if (oppCouldWin) {
            continue skipTry2;
          }
        }
      }

      if (isLegalMove(move, legalMoves)) {
        game.log("Prevent opponent to win by destroying destination: " + getMoveString(move));
        prevWin.push(move.slice());

        let pseudo = makePseudoMove(currBoard, move, MY_ID);
        if (canWin(pseudo, MY_ID, nHeightTowerWin).length > 0) {
          game.log("Prevent opponent to win and possibility for CPU to win in next round: " + getMoveString(move));
          return move;
        }

        // Check if own color could remain on top of source after moving stones
        if (0 === optimal.length) {
          if (MY_ID === parseInt(
            sFrom.substring(sFrom.length - 1 - stones, sFrom.length - stones), 10)) {
              game.log("Prevent opponent to win and own stone on top of source tower: " + getMoveString(move));
              optimal = move.slice();
          }
        }
      }
    }
  }

  if (optimal.length > 0) {
    return optimal;
  }

  if (prevWin.length > 0) {
    let nRand = Math.floor(Math.random() * prevWin.length);
    return prevWin[nRand];
  } else {
    return prevWin;
  }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function isLegalMove(move, legalMoves) {
  let sMove = JSON.stringify(move);
  for (let i = 0; i < legalMoves.length; i++) {
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
  let newBoard = currBoard.slice();

  if (-1 === move[0]) {  // Set stone
    newBoard[move[2]] = nPlayerID.toString();
  } else {  // Move tower
    let sFieldFrom = newBoard[move[0]];
    let sMovedStones = sFieldFrom.substring(sFieldFrom.length - move[1], sFieldFrom.length);
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
  let neighbours = [];
  let nMoves = currBoard[nIndex].length;

  if (0 === nMoves) {
    return neighbours;
  }

  let sField = "";
  for (let dir = 0; dir < DIRS.length; dir++) {
    for (let range = 1; range <= nMoves; range++) {
      sField = currBoard[nIndex + DIRS[dir] * range];
      if (0 !== sField.length && range < nMoves) {  // Route blocked
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
  const sOut = game.getOutside();
  const sPad = game.getPadding();

  let ret = [];
  for (let nIndex = 0; nIndex < currBoard.length; nIndex++) {
    if (0 !== currBoard[nIndex].length &&
      sOut !== currBoard[nIndex] &&
      sPad !== currBoard[nIndex]) {
      let neighbours = checkNeighbourhood(currBoard, nIndex);
      for (let point = 0; point < neighbours.length; point++) {
        let tower = currBoard[(neighbours[point])];
        if ((currBoard[nIndex].length + tower.length >= nHeightTowerWin) &&
          nPlayerID === parseInt(tower[tower.length - 1], 10)) {  // Top = own
          let move = [];  // From, num of stones, to
          move.push(neighbours[point]);
          move.push(tower.length);
          move.push(nIndex);
          ret.push(move);  // Generate list of all opponent winning moves

          if (nPlayerID === MY_ID) {  // Return first found move for CPU to win
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
  let nTop = game.getHeightToWin() * (2 * game.getHeightToWin() + game.getBoardDimension()[0]);
  let nFirst = nTop + game.getHeightToWin();
  let nLeftRight = 2 * game.getHeightToWin() * (Math.floor(nIndex / (game.getBoardDimension()[0] + 2 * game.getHeightToWin())) - game.getHeightToWin());
  return nIndex - nFirst - nLeftRight;
}

function getCoordinateFromField(nField) {
  let x = nField % game.getBoardDimension()[0];
  let y = Math.floor(nField / game.getBoardDimension()[0]);
  return [x, y];
}

function getStringCoordinateFromIndex(nIndex) {
  let coord = getCoordinateFromField(getFieldFromIndex(nIndex));
  return String.fromCharCode(coord[0] + 65) + (coord[1] + 1);
}

function getMoveString(move) {
  if (-1 === move[0]) {
    return "-1:" + move[1] + "-" + getStringCoordinateFromIndex(move[2]);
  }
  return getStringCoordinateFromIndex(move[0]) + ":" + move[1] + "-" + getStringCoordinateFromIndex(move[2]);
}
