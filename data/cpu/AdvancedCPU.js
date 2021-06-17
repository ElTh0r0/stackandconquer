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
 *   cpu.getID();
 *   cpu.getNumOfPlayers();
 *   cpu.getHeightToWin();
 *   cpu.getBoardDimension();
 *   cpu.getOutside();
 *   cpu.getPadding();
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
    return this.#sOut
  }
  getPadding() {
    return this.#sPad
  }
}
cpu = new DebugCPU();

let jsboard = "[\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"2\",\"\",\"\",\"\",\"\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"1\",\"\",\"2\",\"\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"2\",\"2\",\"\",\"\",\"\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"\",\"\",\"\",\"\",\"\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"21\",\"\",\"\",\"\",\"1111\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\",\"-\"]";
let jsmoves = JSON.stringify([[96,1,80],[-1,1,81],[-1,1,82],[-1,1,83],[-1,1,84],[-1,1,95],[80,1,96],[110,1,96],[111,1,96],[-1,1,97],[-1,1,99],[96,1,110],[111,1,110],[96,1,111],[110,1,111],[-1,1,112],[-1,1,113],[-1,1,114],[-1,1,125],[-1,1,126],[-1,1,127],[-1,1,128],[-1,1,129],[110,1,140],[-1,1,141],[-1,1,142],[-1,1,143]]);
let nDirection = 1;

console.log('CPU script returned: ' + getMoveString(callCPU(jsboard, jsmoves, nDirection)));
*/
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

cpu.log("Loading CPU script 'AdvancedCPU' with player ID " + cpu.getID());

function callCPU(jsonBoard, jsonMoves, nDirection) {
  let board = JSON.parse(jsonBoard);
  // cpu.log("BOARD: " + jsonBoard);
  let legalMoves = JSON.parse(jsonMoves);
  // cpu.log("LEGAL MOVES: " + jsonMoves);
  shuffleArray(legalMoves);

  /*
   * Moving directions factor
   * E.g. 5x5 board, max tower height 5 (padding):
   * -16 -15 -14
   * -1   X    1
   * 14  15   16
   */
  // Global variable
  DIRS = [];
  DIRS.push(-(2 * cpu.getHeightToWin() + cpu.getBoardDimension()[0] + 1));  // -16
  DIRS.push(-(2 * cpu.getHeightToWin() + cpu.getBoardDimension()[0]));      // -15
  DIRS.push(-(2 * cpu.getHeightToWin() + cpu.getBoardDimension()[0] - 1));  // -14
  DIRS.push(-1);  // -1
  DIRS.push(1);   //  1
  DIRS.push(-DIRS[2]);  // 14
  DIRS.push(-DIRS[1]);  // 15
  DIRS.push(-DIRS[0]);  // 16

  if (1 === legalMoves.length) {  // Only one move possible, skip calculation
    cpu.log("CPU has only one possible move left");
    return legalMoves[0];
  }

  cpu.log("Possible moves: " + legalMoves.length);
  if (0 !== legalMoves.length) {
    return chooseMove(board, legalMoves);
  }

  // This line never should be reached!
  cpu.log("ERROR: No legal moves passed to script?!");
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
  const nID = cpu.getID();
  const nNumOfPlayers = cpu.getNumOfPlayers();
  
  const SET_NO_OWN_NEIGHBOUR = 1;
  const SET_NO_NEIGHBOUR = 2;
  const SET_OWN_NEIGHBOUR = 5;
  const MOVE_TOWER_TOP2 = 10;
  const MOVE_TOWER_TOP3 = 20;
  const MOVE_TOWER_TOP4 = 50;
  const COULD_WIN_NEXT_ROUND = 100;

  const DESTROY_OPP_TOWER3 = 30;
  const DESTROY_OPP_TOWER3_NEWTOP = 40;
  const DESTROY_OPP_TOWER4 = 150;
  const DESTROY_OPP_TOWER4_NEWTOP = 200;

  let nScore = -9999;
  let bestmove = [-1, -1, -1];

  let cpuMoveToWin = canWin(currBoard, nID);
  if (0 !== cpuMoveToWin.length) {  // CPU can win
    if (isLegalMove(cpuMoveToWin[0], legalMoves)) {
      cpu.log("CPU can win!");
      return cpuMoveToWin[0];
    }
  }

  // Check if opponent could win on current board
  skipMe: for (let opponentID = 1; opponentID <= nNumOfPlayers; opponentID++) {
    if (opponentID === nID) {  // Skip loop for myself (was already checked in previsous loop)
      continue skipMe;
    }

    let oppWinningMoves = canWin(currBoard, opponentID);
    if (0 !== oppWinningMoves.length) {
      for (let k = 0; k < oppWinningMoves.length; k++) {
        cpu.log("Opponent #" + opponentID + " could win: " + getMoveString(oppWinningMoves[k]));
        let prevWin = preventWin(currBoard, oppWinningMoves[k], legalMoves);
        if (0 !== prevWin.length) {
          cpu.log("Found preventive move");
          return prevWin;
        }
      }

      cpu.log("No move found to prevent opponent to win!!");
    }
  }

  // Store current winning moves to be compared with new board winning moves later
  let canWinMoves = [];
  for (let pID = 1; pID <= nNumOfPlayers; pID++) {
    let tempWinMoves = canWin(currBoard, pID);
    canWinMoves.push(tempWinMoves);
  }

  // Check what could happen after executing one of the legal moves
  skipMove: for (let i = 0; i < legalMoves.length; i++) {
    let newBoard = makePseudoMove(currBoard, legalMoves[i], nID);
    cpu.log("Check PseudoMove " + getMoveString(legalMoves[i]));

    // Check if any opponent could win in *next* round if move will be executed
    for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
      if (playerID !== nID) {
        let movesToWin = canWin(newBoard, playerID);

        if (0 !== movesToWin.length &&  // Opponent could win in *next* round
          JSON.stringify(canWinMoves[playerID - 1]) !== JSON.stringify(movesToWin)) {  // New winning moves found (not already available before on old board)
          cpu.log("Opponent #" + playerID + " could win after excuting move " + getMoveString(legalMoves[i]) + " - skipping this move!");
          continue skipMove;  // Skip move, since it could let to opponent win
        }
      }
    }

    // -------------------------------------------------
    // All "bad" moves have been sorted out above before

    // Check if CPU could win in *next* round
    let cpuToWin = canWin(newBoard, nID);
    if (0 !== cpuToWin.length) {
      if (COULD_WIN_NEXT_ROUND > nScore) {
        nScore = COULD_WIN_NEXT_ROUND;
        bestmove = legalMoves[i];
        cpu.log("CPU" + nID + " COULD_WIN_NEXT_ROUND " + getMoveString(legalMoves[i]) + " - score: " + nScore);
      }
    }

    if (-1 === legalMoves[i][0]) {  // Set stone on empty field
      let newNeighbours = checkNeighbourhood(newBoard, legalMoves[i][2]);

      if (0 === newNeighbours.length) {  // No neighbours at all
        if (SET_NO_NEIGHBOUR > nScore) {
          nScore = SET_NO_NEIGHBOUR;
          bestmove = legalMoves[i];
          cpu.log("SET_NO_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
        }
      }

      for (let point = 0; point < newNeighbours.length; point++) {  // Check new neighbours
        let neighbourTower = newBoard[(newNeighbours[point])];

        if (nID === parseInt(neighbourTower[neighbourTower.length - 1], 10)) {  // Neighbour = own
          if (SET_OWN_NEIGHBOUR > nScore) {
            nScore = SET_OWN_NEIGHBOUR;
            bestmove = legalMoves[i];
            cpu.log("SET_OWN_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        } else {
          if (SET_NO_OWN_NEIGHBOUR > nScore) {
            nScore = SET_NO_OWN_NEIGHBOUR;
            bestmove = legalMoves[i];
            cpu.log("SET_NO_OWN_NEIGHBOUR " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        }
      }
    } else {  // Move tower
      let sFrom = currBoard[legalMoves[i][0]];
      let nNum = legalMoves[i][1];
      let sTo = currBoard[legalMoves[i][2]];

      // Move tower and own color is on top of new tower
      if (nID === parseInt(sFrom[sFrom.length - 1], 10)) {
        if (2 === (nNum + sTo.length)) {  // Build stack of 2 stone
          if (MOVE_TOWER_TOP2 > nScore) {
            nScore = MOVE_TOWER_TOP2;
            bestmove = legalMoves[i];
            cpu.log("MOVE_TOWER_TOP2 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        } else if (3 === (nNum + sTo.length)) {  // Build stack of 3 stone
          if (MOVE_TOWER_TOP3 > nScore) {
            nScore = MOVE_TOWER_TOP3;
            bestmove = legalMoves[i];
            cpu.log("MOVE_TOWER_TOP3 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        } else if (4 === (nNum + sTo.length)) {  // Build stack of 4 stone
          if (MOVE_TOWER_TOP4 > nScore) {
            nScore = MOVE_TOWER_TOP4;
            bestmove = legalMoves[i];
            cpu.log("MOVE_TOWER_TOP4 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
          }
        }
      }

      // Move tower and opponent color is on top of new tower
      if (nID !== parseInt(sFrom[sFrom.length - 1], 10)) {
        let sNewFrom = sFrom.substring(sFrom.length - nNum, sFrom.length);

        if (4 === sFrom.length &&       // Destroy opponent tower with height 4
          4 !== (nNum + sTo.length)) {  // and new tower is not height 4 as well!
          if ("" !== sNewFrom) {
            if (nID !== parseInt(sNewFrom[sNewFrom.length - 1], 10)) {  // On from remains opponont stone on top
              if (DESTROY_OPP_TOWER4 > nScore) {
                nScore = DESTROY_OPP_TOWER4;
                bestmove = legalMoves[i];
                cpu.log("DESTROY_OPP_TOWER4 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            } else {  // On from remains own stone on top
              if (DESTROY_OPP_TOWER4_NEWTOP > nScore) {
                nScore = DESTROY_OPP_TOWER4_NEWTOP;
                bestmove = legalMoves[i];
                cpu.log("DESTROY_OPP_TOWER4_NEWTOP " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            }
          }
        } else if (3 === sFrom.length &&  // Destroy opponent tower with height 3
          (nNum + sTo.length) < 3) {      // and new tower is not height 3 or heigher!
          if ("" !== sNewFrom) {
            if (nID !== parseInt(sNewFrom[sNewFrom.length - 1], 10)) {  // On from remains opponont stone on top
              if (DESTROY_OPP_TOWER3 > nScore) {
                nScore = DESTROY_OPP_TOWER3;
                bestmove = legalMoves[i];
                cpu.log("DESTROY_OPP_TOWER3 " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            } else {  // On from remains own stone on top
              if (DESTROY_OPP_TOWER3_NEWTOP > nScore) {
                nScore = DESTROY_OPP_TOWER3_NEWTOP;
                bestmove = legalMoves[i];
                cpu.log("DESTROY_OPP_TOWER3_NEWTOP " + getMoveString(legalMoves[i]) + " - score: " + nScore);
              }
            }
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
    bestmove = legalMoves[nRand];
    cpu.log("No best move found, choosing randome move: " + getMoveString(bestmove));
  } else {
    cpu.log("Best score: " + nScore + " - move: " + getMoveString(bestmove));
  }

  return bestmove;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function preventWin(currBoard, moveToWin, legalMoves) {
  const nID = cpu.getID();
  const nNumOfPlayers = cpu.getNumOfPlayers();
  const nBoardDimensionsX = cpu.getBoardDimension()[0];

  let prevWin = [];
  let pointFrom = moveToWin[0];
  let nNumber = moveToWin[1];
  let pointTo = moveToWin[2];

  // Check if a blocking towers in between can be placed
  let route = pointTo - pointFrom;
  for (let dir = 0; dir < DIRS.length; dir++) {
    if (1 === Math.abs(DIRS[dir])) {  // +1 / -1
      if (Math.abs(route) < (nBoardDimensionsX - 2) &&
        (route > 0 && DIRS[dir] < 0 ||
          route < 0 && DIRS[dir] > 0)) {
        let move = [];
        move.push(-1);
        move.push(1);
        move.push(pointTo + DIRS[dir]);

        let oppCouldWin = false;
        for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
          if (playerID !== nID && !oppCouldWin) {
            let newBoard = makePseudoMove(currBoard, move, nID);
            oppCouldWin = (canWin(newBoard, playerID).length > 0);
          }
        }
        if (isLegalMove(move, legalMoves) && !oppCouldWin) {
          prevWin.push(move);
        }
      }
    } else {
      if (0 === route % DIRS[dir] &&      // There is a route between points
        (route > 0 && DIRS[dir] < 0 ||    // If route pos., dir has to be neg.
          route < 0 && DIRS[dir] > 0)) {  // If route neg., dir has to be pos.
        let moves = route / DIRS[dir];
        // There is more than one field in between
        if (Math.abs(moves) > 1) {
          let move = [];
          move.push(-1);
          move.push(1);
          move.push(pointTo + DIRS[dir]);

          let oppCouldWin = false;
          for (let playerID = 1; playerID <= nNumOfPlayers; playerID++) {
            if (playerID !== nID && !oppCouldWin) {
              let newBoard = makePseudoMove(currBoard, move, nID);
              oppCouldWin = (canWin(newBoard, playerID).length > 0);
            }
          }
          if (isLegalMove(move, legalMoves) && !oppCouldWin) {
            prevWin.push(move);
          }
        }

        // TODO(x): Try to move tower to prevent win
      }
    }
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

function canWin(currBoard, nPlayerID) {
  const nID = cpu.getID();
  const nHeightTowerWin = cpu.getHeightToWin();
  const sOut = cpu.getOutside();
  const sPad = cpu.getPadding();

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

function getFieldFromIndex(nIndex) {
  const nHeightTowerWin = cpu.getHeightToWin();
  const nBoardDimensionsX = cpu.getBoardDimension()[0];
  let nTop = nHeightTowerWin * (2 * nHeightTowerWin + nBoardDimensionsX);
  let nFirst = nTop + nHeightTowerWin;
  let nLeftRight = 2 * nHeightTowerWin * (Math.floor(nIndex / (nBoardDimensionsX + 2 * nHeightTowerWin)) - nHeightTowerWin);
  return nIndex - nFirst - nLeftRight;
}

function getCoordinateFromField(nField) {
  const nBoardDimensionsX = cpu.getBoardDimension()[0];
  let x = nField % nBoardDimensionsX;
  let y = Math.floor(nField / nBoardDimensionsX);
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
