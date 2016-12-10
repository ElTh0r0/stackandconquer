/**
 * \file DummyCPU.js
 *
 * \section LICENSE
 *
 * Copyright (C) 2015-2016 Thorsten Roth <elthoro@gmx.de>
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
 * nNumOfFields
 */

cpu.log("Loading CPU script DummyCPU...")

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

function makeMove(bStonesLeft) {
  board = JSON.parse(jsboard);
  //cpu.log("[0][0][0]: " + board[0][0][0]);
  //cpu.log("[1][0].length: " + board[1][0].length);
  
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

function setRandom() {
  // Seed random?
  do {
    nRandX = Math.floor(Math.random() * 5);
    nRandY = Math.floor(Math.random() * 5);
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
