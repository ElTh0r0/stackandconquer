/**
 * \file DummyCPU.js
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
 * Dummy CPU opponent.
 *
 * Variables provided externally from game:
 * jsboard
 * jsmoves
 * nID (1 or 2 = player 1 / player 2)
 * nBoardDimensionsX
 * nBoardDimensionsY
 * nHeightTowerWin
 * nNumOfPlayers
 * nDirection
 * sOut
 * sPad
 */

cpu.log("Loading CPU script AdvancedDummyCPU...");

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function callCPU() {
  let board = JSON.parse(jsboard);
  cpu.log("BOARD: " + jsboard);
  let legalMoves = JSON.parse(jsmoves);
  cpu.log("LEGAL MOVES: " + jsmoves);
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
  DIRS.push(-(2 * nHeightTowerWin + nBoardDimensionsX + 1));  // -16
  DIRS.push(-(2 * nHeightTowerWin + nBoardDimensionsX));      // -15
  DIRS.push(-(2 * nHeightTowerWin + nBoardDimensionsX - 1));  // -14
  DIRS.push(-1);  // -1
  DIRS.push(1);   //  1
  DIRS.push(-DIRS[2]);  // 14
  DIRS.push(-DIRS[1]);  // 15
  DIRS.push(-DIRS[0]);  // 16

  /*
  cpu.log("board[112].length: " + board[112].length);
  cpu.log("board[112][0]: " + board[112][0]);
  cpu.log("board[113].length: " + board[113].length);
  cpu.log("board[113][0]: " + board[113][0]);
  */

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
  const SETONEMPTY = 1;
  const MOVETOWERTOP = 10;

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

    // Check if CPU could win in *next* round
    let cpuToWin = canWin(newBoard, nID);
    if (0 !== cpuToWin.length) {
      cpu.log("CPU" + nID + " could win after NEXT round, placing " + getMoveString(legalMoves[i]));
      return legalMoves[i];
    }

    // Check if any opponent could win in next round if move will be executed
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

    if (-1 === legalMoves[i][0]) {  // Set stone on empty field
      if (SETONEMPTY > nScore) {
        nScore = SETONEMPTY;
        bestmove = legalMoves[i];
        cpu.log("SETONEMPTY " + getMoveString(legalMoves[i]))
      }
    } else {  // Move tower
      let from = currBoard[legalMoves[i][0]];
      if (nID === parseInt(from[from.length - 1], 10)) {  // Move tower and own color is on top of new tower
        if (MOVETOWERTOP > nScore) {
          nScore = MOVETOWERTOP;
          bestmove = legalMoves[i];
          cpu.log("MOVETOWERTOP " + getMoveString(legalMoves[i]))
        }
      }
    }
  }

  // No "best" move found, choose random
  if (-1 === bestmove[0] &&
    -1 === bestmove[1] &&
    -1 === bestmove[2]) {
      cpu.log("No best move found, choosing randome move...")
      let nRand = Math.floor(Math.random() * legalMoves.length);
      bestmove = legalMoves[nRand];
  }

  return bestmove;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function preventWin(currBoard, moveToWin, legalMoves) {
  let prevWin = [];
  let pointFrom = moveToWin[0];
  let nNumber = moveToWin[1];
  let pointTo = moveToWin[2];

  // Check if a blocking towers in between can be placed
  let route = pointTo - pointFrom;
  for (let dir = 0; dir < DIRS.length; dir++) {
    if (1 === Math.abs(DIRS[dir])) {  // +1 / -1
      if (Math.abs(route) < (nBoardDimensionsX-2) &&
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
      if (0 === route % DIRS[dir] &&       // There is a route between points
          (route > 0 && DIRS[dir] < 0 ||   // If route pos., dir has to be neg.
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
    let sMovedStones = sFieldFrom.substring(sFieldFrom.length - move[1], sFieldFrom.length)
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
      sField = currBoard[nIndex + DIRS[dir]*range];
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

function canWin(currBoard, nPlayerID) {
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
  let nTop = nHeightTowerWin * (2*nHeightTowerWin + nBoardDimensionsX);
  let nFirst = nTop + nHeightTowerWin;
  let nLeftRight = 2*nHeightTowerWin * (Math.floor(nIndex/(nBoardDimensionsX+2*nHeightTowerWin))-nHeightTowerWin);
  return nIndex - nFirst - nLeftRight;
}

function getCoordinateFromField(nField) {
  let x = nField % nBoardDimensionsX;
  let y = Math.floor(nField / nBoardDimensionsX);
  return [x, y];
}

function getStringCoordinateFromIndex(nIndex) {
  let coord = getCoordinateFromField(getFieldFromIndex(nIndex));
  return String.fromCharCode(coord[0] + 65) + (coord[1]+1);
}

function getMoveString(move) {
  if (-1 === move[0]) {
    return "-1:" + move[1] + "-" + getStringCoordinateFromIndex(move[2]);
  }
  return getStringCoordinateFromIndex(move[0]) + ":" + move[1] + "-" + getStringCoordinateFromIndex(move[2]);
}
