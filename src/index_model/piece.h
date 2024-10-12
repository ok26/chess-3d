#ifndef CHESS_PIECE_H
#define CHESS_PIECE_H

#define EMPTY 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

#define BLACK 0
#define WHITE 1

#define MOVED 1
#define EN_PASSANT 2

class Piece {

	int piece;
public:
	
	Piece(int type, int color, int flags) {
		piece = ((flags & 0x03) << 4) | ((color & 0x01) << 3) | (type & 0x07);
	}

	Piece() {
		piece = EMPTY;
	}

	void operator=(Piece a) { piece = a.piece; }

	int getType() const { return piece & 0x07; }
	int getColor() const { return (piece >> 3) & 0x01; }
	int getFlags() const { return (piece >> 4) & 0x03; }

	void setType(int type) { piece &= ~0x07; piece |= type & 0x07; }
	void setColor(int color) { piece &= ~0x08; piece |= (color & 0x01) << 3; }
	void setFlags(int flags) { piece &= ~0x30; piece |= (flags & 0x03) << 4; }

	bool hasMoved() const { return this->getFlags() & MOVED; }
	bool canBeCapturedEP() const { return this->getFlags() & EN_PASSANT; }

	bool operator==(Piece a) const { return (piece & 0x3f) == (a.piece & 0x3f); }
	bool operator!=(Piece a) const { return (piece & 0x3f) != (a.piece & 0x3f); }
};

#endif