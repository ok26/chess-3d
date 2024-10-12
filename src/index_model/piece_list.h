#ifndef PIECE_LIST_H
#define PIECE_LIST_H

#include <iostream>

#include "index_model/piece.h"

class PieceList {

	int board[64];
public:

	int nPieces[2] = { 0, 0 };
	int nSpecPieces[2][6] = { {0,0,0,0,0,0}, {0,0,0,0,0,0} };
	int pieces[2][16];
	int kingSquare[2];

	int operator [](const int i) const { return board[i]; }
	int& operator [](const int i) { return board[i]; }

	void resetPieceLists() {
		nPieces[WHITE] = 0;
		nPieces[BLACK] = 0;
		kingSquare[WHITE] = -1;
		kingSquare[BLACK] = -1;
		for (int i = 0; i < 64; i++)
			board[i] = -1;
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 6; j++)
				nSpecPieces[i][j] = 0;
	}

	int& getKingSquare(int color) { return kingSquare[color]; }

	void addPiece(int square, int color, int type) {
		board[square] = nPieces[color];
		pieces[color][nPieces[color]] = square;
		nPieces[color]++;
		nSpecPieces[color][type - 1]++;
		if (type == KING) kingSquare[color] = square;
	}

	void removePiece(int square, int color, int type) {
		nPieces[color]--;
		nSpecPieces[color][type - 1]--;
		if (nPieces[color] > 0) {
			int index = board[square];
			pieces[color][index] = pieces[color][nPieces[color]];
			board[pieces[color][index]] = index;
		}
		board[square] = -1;
	}

	void movePiece(int fromSquare, int toSquare, bool color) {
		if (fromSquare == kingSquare[WHITE]) kingSquare[WHITE] = toSquare;
		else if (fromSquare == kingSquare[BLACK]) kingSquare[BLACK] = toSquare;

		int index = board[fromSquare];
		board[fromSquare] = -1;
		board[toSquare] = index;
		pieces[color][index] = toSquare;
	}

	void printData() {
		for (int i = 0; i < 2; i++) {
			std::cout << "\n";
			for (int j = 0; j < 6; j++)
				std::cout << nSpecPieces[i][j] << " ";
		}
		std::cout << "\n";
		/*
		std::cout << "WHITE PIECES: ";
		for (int i = 0; i < nPieces[WHITE]; i++)
			std::cout << pieces[WHITE][i] << " ";
		std::cout << "-> " << nPieces[WHITE];
		std::cout << "\nBLACK PIECES: ";
		for (int i = 0; i < nPieces[BLACK]; i++)
			std::cout << pieces[BLACK][i] << " ";
		std::cout << "-> " << nPieces[BLACK];
		std::cout << "\nBOARD\n";
		for (int i = 0; i < 64; i++) {
			if (i != 0 && i % 8 == 0)
				std::cout << "\n";
			std::cout << ((board[i] < 10 && board[i] != -1) ? "  " : " ") << board[i];
		}
		std::cout << "\n\n";*/
	}
};

#endif