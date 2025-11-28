// SPDX-FileCopyrightText: 2024 Maks Verver
// SPDX-License-Identifier: GPL-3.0-or-later

// CPU strength: Hard

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

// Classifies the given list of moves into three types: winning, neutral and
// losing. This is only used by the Monte Carlo player.
function triageMoves(state, moves) {
  const {cfg, fields, nextPlayer} = state;
  const {winningHeight} = cfg;
  const winningMoves = [];
  const neutralMoves = [];
  const losingMoves = [];
  for (const move of moves) {
    if (move.length !== 0 && fields[move[2]].length + move[1] >= winningHeight) {
      const srcField = fields[move[0]];
      if (srcField[srcField.length - 1] === nextPlayer) {
        winningMoves.push(move);
      } else {
        losingMoves.push(move);
      }
    } else {
      neutralMoves.push(move);
    }
  }
  return [winningMoves, neutralMoves, losingMoves];
}

// Plays a mostly-random move, using the following heuristic: play a winning
// move if it exists, don't play an immediately losing move if it can be
// avoided, and otherwise play randomly.
//
// This is only used by the Monte Carlo player.
function playRandomMove(state, allMoves) {
  for (const moves of triageMoves(state, allMoves)) {
    if (moves.length > 0) {
      const choice = randomChoice(moves);
      state._doMoveInternal(choice);
      return;
    }
  }
  // This should never happen, since "pass" is also a neutral move, unless
  // this function is called when the game is already over.
  throw new Error('No moves available!');
}

// Simulates a random playout. Returns the number of moves played.
//
// This is only used by the Monte Carlo player.
function randomPlayout(state, maxSteps) {
  for (let step = 0; step < maxSteps; ++step) {
    const moves = state.generateMoves();
    if (moves.length === 0) return step;  // game over
    playRandomMove(state, moves);
  }
  return maxSteps;
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

// montecarlo.js -- Player implementation based on Monte Carlo simulations.
//
// This is stronger than the "Advanced" CPU in the StackAndConquer distribution
// but weaker than minimax.js. Compared to minimax.js it has the following
// advantages:
//
//  1. It's weaker, which might be more fun for human players.
//  2. It's more random, which which might be more fun for human players.
//  3. It supports games with more than two players.
//
// Like minimax.js, this also supports arbitrary board configurations.
//
// Possibilities for improvements:
//
//  - detect draws due to a sequence of passes (currently, we just simulate
//    for a maximum number of steps)
//  - guide playouts using heuristics (see AdvancedCPU.js for inspiration)
//  - the current version plays until exactly one tower has been captured,
//    which is not optimal when towers-to-win is greater than 1 (idea: when
//    a winning move is found, keep executing winning moves only until
//    none are left, and then count how many points each player scored?)
//

// Number of Monte Carlo simulation steps; higher is better, but slower.
const TOTAL_BUDGET           = 50000;
const MIN_BUDGET_PER_MOVE    =   500;
const MAX_STEPS_TO_SIMULATE  =   250;  // used to break out of ties/loops

function evaluateWithRandomPlayouts(player, initialState, budget) {
  let value = 0.0;
  let errorPrinted = false;
  let simulations = 0;
  while (budget > 0) {
    // Hack: limit number of towers to win to 1, to increase speed and accuracy.
    const state = initialState.clone(1);
    const steps = randomPlayout(state, MAX_STEPS_TO_SIMULATE);
    if (steps >= MAX_STEPS_TO_SIMULATE && !errorPrinted) {
      log('WARNING: maxSteps exceeded!');
      errorPrinted = true;  // only print once
    }
    const winner = state.getWinner();
    value += winner === player ? 1.0 : winner === -1 ? 0.5 : 0.0;
    simulations += 1;
    budget -= steps + 1;
  }
  return value / simulations;
}

// Finds the best move among the given `moves` by simulating random playouts.
function findBestMoves(unusedCfg, state, moves) {
  const triagedMoves = triageMoves(state, moves);
  if (triagedMoves[0].length > 0) {
    // Winning moves available!
    return [triagedMoves[0], 1.0];
  }
  const neutralMoves = triagedMoves[1];
  if (neutralMoves.length === 0) {
    // Only losing moves available.
    return [triagedMoves[2], 0.0];
  }

  const player = state.getNextPlayer();
  const simulationsPerMove = Math.max(MIN_BUDGET_PER_MOVE, Math.floor(TOTAL_BUDGET / neutralMoves.length));
  const bestMoves = [];
  let bestValue = 0.0;
  for (const move of neutralMoves) {
    const undoState = state.doMove(move);
    const value = evaluateWithRandomPlayouts(player, state, simulationsPerMove);
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

// MonteCarloCPU.js -- a CPU player based on Monte Carlo methods.
//

const player = new CpuPlayer(findBestMoves);

function initCPU(jsonBoard) {
  return player.initCpu(jsonBoard);
}

function callCPU(jsonBoard) {
  return player.callCpu(jsonBoard);
}

