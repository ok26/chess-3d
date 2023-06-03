#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "chessPiece.h"
#include "chessMove.h"
#include "moveGenerator.h"
#include "mailbox.h"

#include <string>
#include <map>
#include <sstream>
#include <iostream>

class ChessBoardIndex {

	PieceList pieceList;
	MoveGenerator moveGenerator;
	
	std::map<char, Piece> stringToBoard = {
		{'p', Piece(PAWN,   BLACK, EMPTY)},
		{'P', Piece(PAWN,   WHITE, EMPTY)},
		{'n', Piece(KNIGHT, BLACK, EMPTY)},
		{'N', Piece(KNIGHT, WHITE, EMPTY)},
		{'b', Piece(BISHOP, BLACK, EMPTY)},
		{'B', Piece(BISHOP, WHITE, EMPTY)},
		{'r', Piece(ROOK,   BLACK, EMPTY)},
		{'R', Piece(ROOK,   WHITE, EMPTY)},
		{'q', Piece(QUEEN,  BLACK, EMPTY)},
		{'Q', Piece(QUEEN,  WHITE, EMPTY)},
		{'k', Piece(KING,   BLACK, EMPTY)},
		{'K', Piece(KING,   WHITE, EMPTY)}
	};

	int possibleEpCapture = -1;
	std::pair<Move, Piece> madeMoves[100];
	int nMadeMoves = 0;

public:
	Mailbox mailbox;
	ChessMoves availableMoves;
	int sideToMove;
	int promotedPawnSquare = -1;
	
	void changeBoardState(std::string state) {

		std::istringstream stateParts(state);

		std::string statePart;
		stateParts >> statePart;
		int cur = 0;
		pieceList.resetPieceLists();
		for (int i = 0; i < statePart.length() && cur < 64; i++) {
			if (statePart[i] == '/')
				continue;
			if (statePart[i] < 'A') {
				int j = i;
				while (j - i < (int)(statePart[i] - '0')) {
					mailbox[cur].setType(EMPTY);
					cur++;
					j++;
				}
				continue;
			}
			mailbox[cur] = stringToBoard[statePart[i]];
			pieceList.addPiece(cur, stringToBoard[statePart[i]].getColor());
			cur++;
		}

		stateParts >> statePart;
		sideToMove = statePart == "w" ? WHITE : BLACK;
		possibleEpCapture = -1;
		promotedPawnSquare = -1;
		//Implement rest of Forsyth–Edwards Notation
		moveGenerator.updatePossibleMoves(mailbox, pieceList, availableMoves, sideToMove);
	}

	void printState() {
		for (int i = 0; i < 64; i++) {
			if (i != 0 && i % 8 == 0)
				std::cout << "\n";
			std::cout << mailbox[i].getType() << " ";
		}
		std::cout << "\n";
	}

	void makeMove(Move move, bool waitForSelection = false) {

		madeMoves[nMadeMoves] = { move, (move.isCapture() ? (move.isEpCapture() ? mailbox[possibleEpCapture] : mailbox[move.getTo()]) : Piece()) };
		nMadeMoves++;

		if (possibleEpCapture != -1) {
			mailbox[possibleEpCapture].setFlags(MOVED);
			possibleEpCapture = -1;
		}
		
		Piece& piece = mailbox[move.getFrom()];
		if (move.isDoublePawnPush()) {
				piece.setFlags(EN_PASSANT);
				possibleEpCapture = move.getTo();
		}
		else if (!piece.hasMoved())
			piece.setFlags(MOVED);

		pieceList.movePiece(move, mailbox, sideToMove);
		mailbox.movePiece(move.getFrom(), move.getTo());
		
		if (move.isCastle()) {
			std::pair<int, int> rookMove = mailbox.getRookMoveFromCastle(move);
			mailbox.movePiece(rookMove.first, rookMove.second);
		}
		else if (move.isPromotion()) {
			promotedPawnSquare = move.getTo();
			if (!waitForSelection)
				updatePromotion(move.getFlags() & (~0x4));
		}
		else if (move.isEpCapture())
			mailbox[mailbox.getCapturedEpSquare(move)].setType(EMPTY);
		
		if (!move.isPromotion()) { //Promotion happens first, then moves are updated
			sideToMove ^= WHITE;
			moveGenerator.updatePossibleMoves(mailbox, pieceList, availableMoves, sideToMove);
		}
	}

	void unMakeLastMove() {
		if (nMadeMoves == 0) return;
		nMadeMoves--;
		unMakeMove(madeMoves[nMadeMoves].first, madeMoves[nMadeMoves].second);
	}

	void unMakeMove(Move move, Piece capturedPiece) {
		sideToMove ^= WHITE;
		Move reverseMove = move;
		reverseMove.reverseMove();
		pieceList.movePiece(reverseMove, mailbox, sideToMove);
		mailbox.movePiece(reverseMove.getFrom(), reverseMove.getTo());

		if (move.isCapture()) {
			if (move.isEpCapture()) {
				possibleEpCapture = mailbox.getCapturedEpSquare(move);
				mailbox[possibleEpCapture] = capturedPiece;
				mailbox[possibleEpCapture].setFlags(EN_PASSANT);
				pieceList.addPiece(possibleEpCapture, sideToMove ^ WHITE);
			}
			else {
				mailbox[reverseMove.getFrom()] = capturedPiece;
				pieceList.addPiece(reverseMove.getFrom(), sideToMove ^ WHITE);
			}
		}
		else if (move.isCastle()) {
			std::pair<int, int> rookMove = mailbox.getRookMoveFromCastle(move);
			mailbox.movePiece(rookMove.second, rookMove.first);
		}
		else if (move.isDoublePawnPush()) {
			mailbox[reverseMove.getTo()].setFlags(EMPTY);
			possibleEpCapture = -1;
		}
		else if (move.isPromotion())
			mailbox[reverseMove.getTo()].setType(PAWN);
		
		moveGenerator.updatePossibleMoves(mailbox, pieceList, availableMoves, sideToMove);
	}
	//*************************************************************************
	// Issue is that moves do not remember flags
	// Solution could be to add a structure with info instead of /
	// big pairs, this might change in future but works for now
	// The main, or actually only, issue is the "MOVED" flag
	// --------------------IMPORTANT--------------------------
	// Also remember that the move-unmaking ting is temporary
	// Only the ability to do it BTS should be available
	//*************************************************************************



	void updatePromotion(int promotion) {
		mailbox[promotedPawnSquare].setType(KNIGHT + promotion);
		sideToMove ^= WHITE;
		moveGenerator.updatePossibleMoves(mailbox, pieceList, availableMoves, sideToMove);
		promotedPawnSquare = -1;
	}

	//Very slow but only for visuals
	Move getMove(int fromPos, int toPos) { //Promotions not implemented but should not be a problem as long as this is only used for visuals
		for (int i = 0; i < availableMoves.nMoves; i++) {
			Move& move = availableMoves[i];
			if (move.getFrom() == fromPos && move.getTo() == toPos)
				return move;
		}
	}

	void filterPseudoLegalMoves() {
		for (int i = 0; i < availableMoves.nMoves; i++) {
			Move& move = availableMoves[i];

		}
	}
};

#endif