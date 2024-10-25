/**
 * \file MinimaxCPU.js
 *
 * \section LICENSE
 *
 * Copyright (C) 2024 Maks Verver
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
 *
 * CPU opponent using the Minimax algorithm.
 *
 * This file was auto-generated from multiple source files. The original source
 * code is available here: https://github.com/maksverver/stackandconquer-ai/
 */

'use strict';

// util.js -- a grab bag of utility functions used by other parts of the code.
//
// eslint-disable-next-line no-undef
let logDelegate = typeof game === 'object' ? game.log : console.log;

// Note: unlike console.log(), game.log() supports only a single string argument!
function log(...args) {
  if (args.length !== 1) {
    throw new Error('log() expects exactly 1 argument!');
  }
  logDelegate(args[0]);
}

// Creates a static descriptor of a board configuration.
//
// See the final return value for a list of fields. Most importantly, it the
// `moves` field is a template of moves that is used by State.generateMoves()
// to generate valid moves efficiently.
function createConfig(
    rows, cols, inputFields, outside, padding, paddingSize,
    winningHeight, playerCount) {
  const paddedRows = rows + 2*paddingSize;
  const paddedCols = cols + 2*paddingSize;
  if (inputFields.length !== paddedRows * paddedCols) {
    throw new Error('Invalid length of input fields');
  }
  const apiToFieldIndex = [];
  const fieldIndexToApi = [];
  let fieldCount = 0;
  for (let r1 = 0; r1 < paddedRows; ++r1) {
    for (let c1 = 0; c1 < paddedCols; ++c1) {
      const i = paddedCols * r1 + c1;
      if (inputFields[i] === outside || inputFields[i] === padding) {
        apiToFieldIndex.push(-1);
      } else {
        apiToFieldIndex.push(fieldCount++);
        fieldIndexToApi.push(i);
      }
    }
  }
  if (fieldCount > 30) {
    throw new Error('Too many fields (maximum supported: 30)');
  }

  // moves[dst][height] is an array of [src, mask] fields, where
  // mask is a bitmask of fields between src and dst.
  const moves = [];
  const DR = [-1, -1, -1,  0,  0, +1, +1, +1];
  const DC = [-1,  0, +1, -1, +1, -1,  0, +1];
  for (let r2 = 0; r2 < paddedRows; ++r2) {
    for (let c2 = 0; c2 < paddedCols; ++c2) {
      const j = paddedCols * r2 + c2;
      if (inputFields[j] === outside || inputFields[j] === padding) continue;
      const dst = moves.length;
      moves.push([]);
      for (let height = 0; height < winningHeight; ++height) {
        moves[dst].push([]);
      }
      for (let dir = 0; dir < 8; ++dir) {
        const dr = DR[dir], dc = DC[dir];
        let mask = 0;
        for (let height = 1; height < winningHeight; ++height) {
          const r1 = r2 - dr*height;
          const c1 = c2 - dc*height;
          if (r1 < 0 || r1 >= paddedRows || c1 < 0 || c1 >= paddedCols) break;
          const i = paddedCols * r1 + c1;
          if (inputFields[i] === outside || inputFields[i] === padding) break;
          const src = apiToFieldIndex[i];
          moves[dst][height].push([src, mask]);
          mask |= 1 << src;
        }
      }
    }
  }

  return Object.freeze({
    apiToFieldIndex: Object.freeze(apiToFieldIndex),
    fieldIndexToApi: Object.freeze(fieldIndexToApi),
    fieldCount,
    moves: Object.freeze(moves),
    winningHeight,
    playerCount,
    // These are only used for move parsing/formatting and debug printing:
    rows,
    cols,
    paddingSize,
  });
}

// Converts a compact field index (which must be valid, i.e., an integer between
// 0 and cfg.fieldCount, exclusive) to a [row, col] pair.
function fieldIndexToRowCol(cfg, index) {
  const i = cfg.fieldIndexToApi[index];
  const pad = cfg.paddingSize;
  const rowStride = pad * 2 + cfg.cols;
  const col = i % rowStride;
  const row = (i - col)/rowStride;
  return [row - pad, col - pad];
}

function arrayEquals(a, b) {
  if (a.length !== b.length) return false;
  for (let i = 0; i < a.length; ++i) if (a[i] !== b[i]) return false;
  return true;
}

function indexOfMove(moves, move) {
  for (let i = 0; i < moves.length; ++i) {
    if (arrayEquals(move, moves[i])) return i;
  }
  return -1;
}

function randomChoice(array) {
  return array[Math.floor(Math.random() * array.length)]
}

// formatting.js -- helper functions for formatting moves as text strings.
//
// Strings are of the form: "2a3b4", meaning "move 2 stones from a3 to b4", or
// "e5" meaning "place a new stone on e5".
//
// See parsing.js for the opposite.
//

function formatRow(row) {
  return String(row + 1);
}

function formatCol(col) {
  return String.fromCharCode('a'.charCodeAt(0) + col);
}

function formatField(cfg, field) {
  const [row, col] = fieldIndexToRowCol(cfg, field);
  return formatCol(col) + formatRow(row);
}

function formatMove(cfg, move) {
  if (move.length === 0) return 'pass';
  const [src, cnt, dst] = move;
  let res = '';
  if (cnt !== 1) res += String(cnt);
  if (src !== -1) res += formatField(cfg, src);
  res += formatField(cfg, dst);
  return res;
}

function formatMoves(cfg, moves) {
  return moves.map(move => formatMove(cfg, move)).join(' ');
}

// gamestate.js -- mutable state representation of a game in progress
//

// Move that represents passing (an empty array).
const PASS = Object.freeze([]);

function cloneFields(fields) {
  return fields.map(pieces => pieces.slice());
}

class State {

  constructor(cfg, fields, nextPlayer, lastMove, piecesLeft, scoresLeft, occupied) {
    // Game configuration object as returned by createConfig()
    this.cfg = cfg;
    // Array of fields. Each field is an array with pieces (numbers 0 through 2)
    this.fields = fields;
    // Next player to move (number 0 through 2)
    this.nextPlayer = nextPlayer;
    // Last move played (to prevent reverting, which is illegal) or null
    this.lastMove = lastMove;
    // Array of number of pieces left to place, per player
    this.piecesLeft = piecesLeft;
    // Number of towers each player still needs to conquery to win
    this.scoresLeft = scoresLeft;
    // Bitmask of occupied fields
    this.occupied = occupied;
  }

  getNextPlayer() {
    return this.nextPlayer;
  }

  // Returns the player index of the winner, or -1 if there is no winner.
  getWinner() {
    return this.scoresLeft.indexOf(0);
  }

  _incNextPlayer() {
    ++this.nextPlayer;
    if (this.nextPlayer === this.cfg.playerCount) this.nextPlayer = 0;
}

  _decNextPlayer() {
    if (this.nextPlayer === 0) this.nextPlayer = this.cfg.playerCount;
    --this.nextPlayer;
  }

  _doMoveInternal(move) {
    // This implementation is essentially reversed in undoMove(). Keep the
    // implementations in sync.
    let removed = null;
    if (move.length === 0) {
      // Pass. Play returns to the *previous* player (which is the same as the
      // next player in a 2-player game).
      this._decNextPlayer();
      // Open question: should we update lastMove in this case?
    } else {
      const [src, cnt, dst] = move;
      const dstField = this.fields[dst];
      if (src === -1) {
        const player = this.nextPlayer;
        --this.piecesLeft[player];
        dstField.push(player);
        this.occupied ^= 1 << dst;
      } else {
        const srcField = this.fields[src];
        dstField.push(...srcField.splice(srcField.length - cnt));
        if (srcField.length === 0) {
          this.occupied ^= 1 << src;
        }
        if (dstField.length >= this.cfg.winningHeight) {
          removed = dstField.splice(0);
          const winner = removed[removed.length - 1];
          this.scoresLeft[winner] -= 1;
          this.occupied ^= 1 << dst;
          for (const player of removed) ++this.piecesLeft[player];
        }
      }
      this._incNextPlayer();
      this.lastMove = move;
    }
    return removed;
  }

  // Executes a move and returns undo state that can be passed to undoMove() to
  // undo the move.
  //
  // Important: `move` must be valid!
  doMove(move) {
    return [this.lastMove, this._doMoveInternal(move)];
  }

  // Undoes the last move.
  //
  // Important: `move` must be the last move done, and `undoState` must be the
  // unmodified object return by the corresponding call to `doMove()`.
  undoMove(move, undoState) {
    // This implementation is essentially the same as doMoveInternal(), but in
    // reverse. Keep the implementations in sync.
    if (move.length === 0) {
      this._incNextPlayer();
    } else {
      this.lastMove = undoState[0];
      this._decNextPlayer();
      const [src, cnt, dst] = move;
      const dstField = this.fields[dst];
      if (src === -1) {
        ++this.piecesLeft[this.nextPlayer];
        dstField.pop();
        this.occupied ^= 1 << dst;
      } else {
        const srcField = this.fields[src];
        const removed = undoState[1];
        if (removed != null) {
          for (const player of removed) --this.piecesLeft[player];
          const winner = removed[removed.length - 1];
          this.scoresLeft[winner] += 1;
          dstField.push(...removed);
          this.occupied ^= 1 << dst;
        }
        if (srcField.length === 0) {
          this.occupied ^= 1 << src;
        }
        srcField.push(...dstField.splice(dstField.length - cnt));
      }
    }
  }

  // Generates a list of all possible moves.
  //
  // A move is a triple [src, cnt, dst], or an empty array [] to pass.
  // If src === -1 and cnt === 1 and a new piece is placed.
  //
  // Rules of the game:
  //  - https://spielstein.com/games/mixtour/rules (2 players)
  //  - https://spielstein.com/games/mixtour/rules/a-trois (3 players)
  generateMoves() {
    if (this.getWinner() !== -1) return [];  // Game is over
    const {cfg, fields, occupied, nextPlayer, lastMove, piecesLeft} = this;
    const {moves: moveTemplates} = cfg;
    const moves = [];
    let lastSrc = -1, lastCnt = 0, lastDst = -1;
    if (lastMove != null && lastMove.length != 0) {
      [lastSrc, lastCnt, lastDst] = lastMove;
    }
    for (let dst = 0; dst < fields.length; ++dst) {
      const dstHeight = fields[dst].length;
      if (dstHeight === 0) {
        if (piecesLeft[nextPlayer]) {
          moves.push([-1, 1, dst]);  // place new piece
        }
      } else {
        const options = moveTemplates[dst][dstHeight];
        for (let i = 0; i < options.length; ++i) {
          const src = options[i][0];
          const srcHeight = fields[src].length;
          if (srcHeight !== 0 && (occupied & options[i][1]) === 0) {
            for (let cnt = 1; cnt <= srcHeight; ++cnt) {
              // Do not allow undoing the last move.
              if (src == lastDst && cnt === lastCnt && dst == lastSrc) continue;
              moves.push([src, cnt, dst]);  // move pieces
            }
          }
        }
      }
    }
    if (moves.length === 0) moves.push(PASS);
    return moves;
  }

  // Returns the state as a JSON-serializable object. This does not do a deep
  // clone, so it's invalidated when the state changes! To prevent this, the
  // caller should serialize the object to a string.
  toJson() {
    return {
      fields: this.fields,
      nextPlayer: this.nextPlayer,
      lastMove: this.lastMove,
      scoresLeft: this.scoresLeft,
      piecesLeft: this.piecesLeft,
    };
  }

  clone(winningScore) {
    const scoresLeft = this.scoresLeft.slice();
    if (winningScore != null) scoresLeft.fill(winningScore);
    return new State(
      this.cfg,
      cloneFields(this.fields),
      this.nextPlayer,
      this.lastMove,
      this.piecesLeft.slice(),
      scoresLeft,
      this.occupied);
  }
}

// Heuristically evaluates the state with respect to the next player, without
// searching more deeply. (Although this function does look for possible moves
// that are immedaitely winning.)
//
// The current evaluation function is not highly optimized. It can probably
// be optimized significantly.
//
// This only works for 2 players and is only used by the Minimax player.
function evaluateImmediately(state) {
  const winner = state.getWinner();
  if (winner !== -1) {
    return winner === state.nextPlayer ? 1000000000 : -1000000000;
  }
  const {cfg, nextPlayer, fields, occupied, scoresLeft} = state;
  const {moves: moveTemplates, winningHeight} = cfg;
  let score = 10000 * (scoresLeft[1 - nextPlayer] - scoresLeft[nextPlayer]);
  for (let dst = 0; dst < fields.length; ++dst) {
    const dstField = fields[dst];
    const dstHeight = dstField.length;
    if (dstHeight > 0) {
      const options = moveTemplates[dst][dstHeight];
      for (const [src, mask] of options) {
        const srcField = fields[src];
        const srcHeight = srcField.length;
        if (srcHeight + dstHeight >= winningHeight && (occupied & mask) === 0) {
          if (srcField[srcHeight - 1] === nextPlayer) {
            // Winning move found!
            score += 1000;
          } else {
            // Winning move for opponent (though I might still be able to prevent it).
            // Possible improvement: check if I have moves to prevent it.
            score -= 100;
          }
        }
      }
      // Reward piece on top of a tower.
      if (dstField[dstHeight - 1] === nextPlayer) {
        score += 10 * dstHeight;
      } else {
        score -= 10 * dstHeight;
      }
      // Reward pieces on the board.
      for (let i = 0; i < dstHeight; ++i) {
        if (dstField[i] === nextPlayer) {
          score += 1 + i;
        } else {
          score -= 1 + i;
        }
      }
    }
  }
  return score;
}

// Creates a state from an object in the same format as produced by toJson(),
// defined above.
//
// Note: the new state takes ownership of mutable inputJson members like
// `fields`, `piecesLeft`, and `scoresLeft`, so the caller should have
// deep-cloned the input JSON before passing it to this function, and not use
// the object afterwards.
function createStateFromJson(cfg, inputJson) {
  // Recompute `occupied` from given fields.
  const fields = inputJson.fields;
  let occupied = 0;
  for (let i = 0; i < fields.length; ++i) {
    if (fields[i].length > 0) occupied |= 1 << i;
  }
  return new State(cfg, fields, inputJson.nextPlayer, inputJson.lastMove, inputJson.piecesLeft, inputJson.scoresLeft, occupied);
}

// minimax.js -- Minimax algorithm for game tree search.
//
// This implementation uses the Negamax variant with alpha-beta pruning.
//
// This algorithm supports arbitrary board configurations, but only two players.
// See montecarlo.js for an algorithm that is weaker, but does support more than
// two players.
//
// Opportunities for optimization:
//
//  - move ordering by shallow search
//  - instead of calling generateMoves(), generate moves incrementally, to
//    avoid wasting work when a beta-cutoff happens.
//  - add a transposition table?
//

// Determines the strength of the AI: higher is better, but slower.
const SEARCH_DEPTH = 4;

// Returns a pair of [list of best moves, best value].
function findBestMoves(unusedCfg, state, moves) {
  function search(depthLeft, alpha, beta) {
    if (depthLeft === 0) {
      return evaluateImmediately(state);
    }
    const moves = state.generateMoves();
    if (moves.length === 0) {
      // Game is over. Adjust value by `depthLeft` to reward quicker wins.
      let value = evaluateImmediately(state);
      if (value > 0) value += depthLeft;
      if (value < 0) value -= depthLeft;
      return value;
    }
    let bestValue = -Infinity;
    for (const move of moves) {
      const undoState = state.doMove(move);
      const value = -search(depthLeft - 1, -beta, -alpha);
      state.undoMove(move, undoState);
      if (value > bestValue) {
        bestValue = value;
        if (value > alpha) alpha = value;
        if (value >= beta) break;
      }
    }
    return bestValue;
  }

  const bestMoves = [];
  let bestValue = -Infinity;
  for (const move of moves) {
    const undoState = state.doMove(move);
    const value = -search(SEARCH_DEPTH - 1, -Infinity, -bestValue + 1);
    state.undoMove(move, undoState);
    if (value > bestValue) {
      bestValue = value;
      bestMoves.length = 0;
    }
    if (value === bestValue) {
      bestMoves.push(move);
    }
  }
  return [bestMoves, bestValue];
}

// cpu-player.js -- Wrapper that implements the StackAndConquer CallCPU API.
//

function convertMoveFromApi(cfg, apiMove) {
  const {apiToFieldIndex} = cfg;
  if (apiMove == null || apiMove.length === 0) return null;
  let [src, cnt, dst] = apiMove;
  if (src !== -1) src = apiToFieldIndex[src];
  dst = apiToFieldIndex[dst];
  return [src, cnt, dst];
}

function movesToCanonicalString(cfg, moves) {
  return moves.map(move => formatMove(cfg, move)).sort().join(',');
}

class CpuPlayer {
  constructor(findBestMoves) {
    this.findBestMoves = findBestMoves;
    this.cfg = null;
  }

  initCpu(jsonBoard) {
    if (this.cfg != null) {
      log('WARNING: initCpu() called more than once!');
    }
    this.cfg = createConfig(
      game.getBoardDimensionY(),
      game.getBoardDimensionX(),
      JSON.parse(jsonBoard),
      game.getOutside(),
      game.getPadding(),
      game.getHeightToWin(),  // padding size
      game.getHeightToWin(),
      game.getNumOfPlayers());
  }

  // Reconstructs config and state from jsonBoard and jsonMoves, invokes
  // findBestMoves(cfg, state, moves) to list moves, then returns a move
  // selected at random.
  callCpu(jsonBoard) {
    const cfg = this.cfg;
    if (cfg == null) {
      throw new Error('initCPU() has not been called!');
    }

    const board = JSON.parse(jsonBoard);

    const nextPlayer = game.getID() - 1;
    if (!Number.isInteger(nextPlayer) || nextPlayer < 0 || nextPlayer >= cfg.playerCount) {
      throw new Error('Invalid player ID');
    }

    // Convert legal moves from API format to ensure we never make illegal moves.
    const apiMoves = JSON.parse(game.getLegalMoves());
    if (apiMoves.length === 0) throw new Error('No moves available');
    if (apiMoves.length === 1) return apiMoves[0];
    const moves = apiMoves.map(move => convertMoveFromApi(cfg, move));

    const fields = [];
    for (let i = 0; i < cfg.fieldCount; ++i) {
      const field = [];
      const string = board[cfg.fieldIndexToApi[i]];
      for (let j = 0; j < string.length; ++j) {
        const piece = string.charCodeAt(j) - '1'.charCodeAt(0);
        if (piece < 0 || piece >= cfg.playerCount) throw new Error('Invalid piece');
        field.push(piece);
      }
      fields.push(field);
    }
    if (fields.length !== cfg.fieldCount) throw new Error('Invalid number of fields');

    const state = createStateFromJson(cfg, {
      fields,
      nextPlayer,
      piecesLeft: game.getNumberOfStones(),
      scoresLeft: game.getTowersNeededToWin(),
      lastMove: convertMoveFromApi(cfg, game.getLastMove()),
    });

    // For debugging: verify the game's move list matches what I would generate.
    const movesString = movesToCanonicalString(cfg, moves);
    const generatedMovesString = movesToCanonicalString(cfg, state.generateMoves());
    if (movesString !== generatedMovesString) {
      // This currently only happens for the 3-player games. I should sync my
      // implementation once the 3-player rules are fully fleshed out:
      // https://github.com/ElTh0r0/stackandconquer/issues/14
      log('WARNING: API-provided moves differ from generated moves');
      // log('\tConfig:          ' + JSON.stringify(cfg));  // too spammy
      log('State:           ' + JSON.stringify(state.toJson()));
      log('Provided moves:  ' + movesString);
      log('Generated moves: ' + generatedMovesString);
    }

    const [bestMoves, bestValue] = this.findBestMoves(cfg, state, moves);
    const bestMove = randomChoice(bestMoves);
    log(moves.length + ' moves, ' + bestMoves.length + ' best with value ' + bestValue + ': ' +
        formatMoves(cfg, bestMoves) + '; selected ' + formatMove(cfg, bestMove) + ' at random');
    const moveIndex = indexOfMove(moves, bestMove);
    if (moveIndex < 0) throw new Error('Best move not found somehow?!');
    return apiMoves[moveIndex];
  }
}

// MinimaxCPU.js -- a CPU player based on Minimax game tree search.
//

const player = new CpuPlayer(findBestMoves);

function initCPU(jsonBoard) {
  if (game.getNumOfPlayers() !== 2) {
    throw new Error('Unsupported number of players!');
  }
  return player.initCpu(jsonBoard);
}

function callCPU(jsonBoard) {
  return player.callCpu(jsonBoard);
}

