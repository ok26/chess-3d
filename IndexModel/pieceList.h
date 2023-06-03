#ifndef PIECE_LIST_H
#define PIECE_LIST_H

#include "chessPiece.h"
#include "chessMove.h"
#include "mailbox.h"

class PieceList {

	int board[64];
public:

	int nWhitePieces = 0, nBlackPieces = 0;
	int whitePieces[16], blackPieces[16];

	void resetPieceLists() {
		nWhitePieces = 0;
		nBlackPieces = 0;
		for (int i = 0; i < 64; i++)
			board[i] = -1;
	}

	void movePiece(Move move, Mailbox mailbox, bool color) {
		int fromSquare = move.getFrom();
		int toSquare = move.getTo();
		
		if (move.isCapture()) {
			int(&oppPieceList)[16] = (color == WHITE ? this->blackPieces : this->whitePieces);
			int& nOppPieces = (color == WHITE ? this->nBlackPieces : this->nWhitePieces);

			int capturedSquare;
			if (move.isEpCapture())
				capturedSquare = mailbox.getCapturedEpSquare(move);
			else
				capturedSquare = move.getTo();
			
			nOppPieces--;
			if (nOppPieces > 0) {
				int index = board[capturedSquare];
				oppPieceList[index] = oppPieceList[nOppPieces];
				board[oppPieceList[index]] = index;
			}
			board[capturedSquare] = -1;
			move.setFlags(QUIET_MOVE);
			movePiece(move, mailbox, color);
		}
		else {

			if (move.isCastle()) {
				std::pair<int, int> rookMove = mailbox.getRookMoveFromCastle(move);
				Move move(rookMove.first, rookMove.second, QUIET_MOVE);
				movePiece(move, mailbox, color);
			}

			//Normal move
			int(&pieceList)[16] = (color == WHITE ? this->whitePieces : this->blackPieces);
			int index = board[fromSquare];

			board[fromSquare] = -1;
			board[toSquare] = index;
			pieceList[index] = toSquare;

			/*
			std::cout << "WHITE PIECES: ";
			for (int i = 0; i < nWhitePieces; i++)
				std::cout << whitePieces[i] << " ";
			std::cout << "-> " << nWhitePieces;
			std::cout << "\nBLACK PIECES: ";
			for (int i = 0; i < nBlackPieces; i++)
				std::cout << blackPieces[i] << " ";
			std::cout << "-> " << nBlackPieces;
			std::cout << "\nBOARD\n";
			for (int i = 0; i < 64; i++) {
				if (i != 0 && i % 8 == 0)
					std::cout << "\n";
				std::cout << ((board[i] < 10 && board[i] != -1) ? "  " : " ") << board[i];
			}
			std::cout << "\n\n";
			//*/
		}
	}

	void addPiece(int square, int color) {
		if (color == WHITE) {
			board[square] = nWhitePieces;
			whitePieces[nWhitePieces] = square;
			nWhitePieces++;
		}
		else {
			board[square] = nBlackPieces;
			blackPieces[nBlackPieces] = square;
			nBlackPieces++;
		}
	}

	void removePiece(int square, int color) {
		//Implement
	}
};

#endif