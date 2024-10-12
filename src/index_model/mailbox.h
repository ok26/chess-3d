#ifndef MAILBOX_H
#define MAILBOX_H

#include <utility>

#include "index_model/piece.h"
#include "index_model/move.h"

class Mailbox {

	int mailbox[120] = {
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
		 -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
		 -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
		 -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
		 -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
		 -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
		 -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
		 -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	int mailbox64[64] = {
		21, 22, 23, 24, 25, 26, 27, 28,
		31, 32, 33, 34, 35, 36, 37, 38,
		41, 42, 43, 44, 45, 46, 47, 48,
		51, 52, 53, 54, 55, 56, 57, 58,
		61, 62, 63, 64, 65, 66, 67, 68,
		71, 72, 73, 74, 75, 76, 77, 78,
		81, 82, 83, 84, 85, 86, 87, 88,
		91, 92, 93, 94, 95, 96, 97, 98
	};

	Piece chessBoard[64];

public:

	Piece operator [](const int i) const {
		return chessBoard[i];
	}

	Piece& operator [](const int i) {
		return chessBoard[i];
	}

	int getRow(int square) { return square / 8; }
	int getColumn(int square) { return square % 8; }
	int getSquareWithOffset(int fromSquare, int offset) { return mailbox[mailbox64[fromSquare] + offset]; }

	std::pair<int, int> getRookMoveFromCastle(Move move) {
		if (move.isQueenCastle()) return { move.getFrom() - 4, move.getFrom() - 1 };
		else return { move.getFrom() + 3, move.getFrom() + 1 };
	}
	int getCapturedEpSquare(Move move) { return move.getFrom() + ((move.getTo() % 8) - (move.getFrom() % 8)); }

	void movePiece(int fromSquare, int toSquare) {
		chessBoard[toSquare] = chessBoard[fromSquare];
		chessBoard[fromSquare].setType(EMPTY);
	}
};

#endif