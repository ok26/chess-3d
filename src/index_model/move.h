#ifndef CHESS_MOVE_H
#define CHESS_MOVE_H

#include "index_model/piece.h"

#define QUIET_MOVE				0b0000
#define DOUBLE_PAWN_PUSH		0b0001
#define KING_CASTLE				0b0010
#define QUEEN_CASTLE			0b0011
#define CAPTURE					0b0100
#define EP_CAPTURE				0b0101
#define KNIGHT_PROMOTION		0b1000
#define BISHOP_PROMOTION		0b1001
#define ROOK_PROMOTION			0b1010
#define QUEEN_PROMOTION			0b1011
#define KNIGHT_PROMOTION_CAP	0b1100
#define BISHOP_PROMOTION_CAP	0b1101
#define ROOK_PROMOTION_CAP		0b1110
#define QUEEN_PROMOTION_CAP		0b1111

const int MAX_AVAILABLE_MOVES = 300;
const int MAX_GAME_MOVES = 1000;

class Move {

	int move;
public:

	Move(int from, int to, int flags) {
		move = ((flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
	}
	Move() {
		move = 0;
	}

	void operator=(Move a) { move = a.move; }

	int getTo() const { return move & 0x3f; }
	int getFrom() const { return (move >> 6) & 0x3f; }
	int getFlags() const { return (move >> 12) & 0x0f; }

	void setTo(int to) { move &= ~0x3f; move |= to & 0x3f; }
	void setFrom(int from) { move &= ~0xfc0; move |= (from & 0x3f) << 6; }
	void setFlags(int flags) { move &= ~0xf000; move |= (flags & 0xf) << 12; }

	bool isCapture() const { return this->getFlags() & CAPTURE; }
	bool isPromotion() const { return this->getFlags() & (1 << 3); }
	bool isEpCapture() const { return this->getFlags() == EP_CAPTURE; }
	bool isCastle() const { return (this->getFlags() | 1) == QUEEN_CASTLE; }
	bool isDoublePawnPush() const { return this->getFlags() == DOUBLE_PAWN_PUSH; }
	bool isQueenCastle() const { return this->getFlags() == QUEEN_CASTLE; }
	bool isKingCastle() const { return this->getFlags() == KING_CASTLE; }

	bool operator==(Move a) const { return (move & 0xffff) == (a.move & 0xffff); }
	bool operator!=(Move a) const { return (move & 0xffff) != (a.move & 0xffff); }
};


class ChessMoves {

	Move moves[MAX_AVAILABLE_MOVES];
public:
	int nMoves = 0;

	Move operator [](const int i) const { return moves[i]; }
	Move& operator [](const int i) { return moves[i]; }

	void resetMoves() { nMoves = 0; }

	void addMove(int from, int to, int flags) {
		moves[nMoves] = Move(from, to, flags);
		nMoves++;
	}

	void removeMove(int index) {
		if (index < 0 || index >= nMoves || nMoves == 0) return;
		nMoves--;
		moves[index] = moves[nMoves];
	}
};


struct MadeMove {
	Move move;
	int movedPieceFlags;
	Piece capturedPiece;
	int previousEpCapture;
	int halfMoveClock;
};

class MadeMoves {
	MadeMove madeMoves[MAX_GAME_MOVES];
	
public:
	int nMadeMoves = 0;
	int maxMadeMoves = 0;

	void resetMadeMoves() {
		nMadeMoves = 0;
		maxMadeMoves = 0;
	}

	void addMove(MadeMove move) {
		madeMoves[nMadeMoves] = move;
		nMadeMoves++;
		if (maxMadeMoves < nMadeMoves) maxMadeMoves = nMadeMoves;
	}

	void popLastMove() {
		nMadeMoves--;
	}

	MadeMove getLastMove() {
		if (nMadeMoves == 0) return MadeMove();
		return madeMoves[nMadeMoves - 1];
	}

	MadeMove getNextMove() {
		if (nMadeMoves == maxMadeMoves) return MadeMove();
		return madeMoves[nMadeMoves];
		nMadeMoves++;
	}

	void updateLastPromotionMove(int promotion) {
		Move& move = madeMoves[nMadeMoves - 1].move;
		move.setFlags(move.getFlags() | promotion);
	}
};



#endif