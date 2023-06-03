#ifndef CHESS_MOVE_H
#define CHESS_MOVE_H

#define QUIET_MOVE 0
#define DOUBLE_PAWN_PUSH 1
#define KING_CASTLE 2
#define QUEEN_CASTLE 3
#define CAPTURE 4
#define EP_CAPTURE 5
#define KNIGHT_PROMOTION 8
#define BISHOP_PROMOTION 9
#define ROOK_PROMOTION 10
#define QUEEN_PROMOTION 11
#define KNIGHT_PROMOTION_CAP 12
#define BISHOP_PROMOTION_CAP 13
#define ROOK_PROMOTION_CAP 14
#define QUEEN_PROMOTION_CAP 15

const int MAX_MOVES = 300;

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

	void reverseMove() {
		int fromSquare = this->getFrom();
		this->setFrom(this->getTo());
		this->setTo(fromSquare);
		this->setFlags(QUIET_MOVE); // Special moves have to be made independently 
	}

	bool isCapture() const { return this->getFlags() & CAPTURE; }
	bool isPromotion() const { return this->getFlags() & (1 << 3); }
	bool isEpCapture() const { return this->getFlags() == EP_CAPTURE; }
	bool isCastle() const { return (this->getFlags() | 1) == QUEEN_CASTLE; }
	bool isDoublePawnPush() const { return this->getFlags() == DOUBLE_PAWN_PUSH; }

	bool operator==(Move a) const { return (move & 0xffff) == (a.move & 0xffff); }
	bool operator!=(Move a) const { return (move & 0xffff) != (a.move & 0xffff); }
};


class ChessMoves {

	Move moves[MAX_MOVES];
public:
	int nMoves = 0;

	Move operator [](const int i) const { return moves[i]; }
	Move& operator [](const int i) { return moves[i]; }

	void resetMoves() { nMoves = 0; }

	void addMove(int from, int to, int flags) {
		moves[nMoves] = Move(from, to, flags);
		nMoves++;
	}
};



#endif