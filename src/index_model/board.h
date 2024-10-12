#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "index_model/piece.h"
#include "index_model/move.h"
#include "index_model/move_gen.h"
#include "index_model/mailbox.h"

#include <string>
#include <map>
#include <sstream>
#include <iostream>

#define CHECKMATE 1
#define STALEMATE 2
#define INSUFFICIENT_MATERIAL 3
#define REPETITION 4
#define MOVE_RULE 5

class ChessBoardIndex {

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
	int filteredMoves[MAX_AVAILABLE_MOVES];

public:
	Mailbox mailbox;
	ChessMoves availableMoves;
	PieceList pieceList;
	MadeMoves madeMoves;
	int sideToMove;
	int promotedPawnSquare = -1;
	int halfMoveClock = 0;
	
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
			pieceList.addPiece(cur, stringToBoard[statePart[i]].getColor(), mailbox[cur].getType());
			cur++;
		}

		stateParts >> statePart;
		sideToMove = statePart == "w" ? WHITE : BLACK;

		stateParts >> statePart;
		if (statePart.find('K') == std::string::npos) mailbox[63].setFlags(MOVED);
		if (statePart.find('k') == std::string::npos) mailbox[7].setFlags(MOVED);
		if (statePart.find('Q') == std::string::npos) mailbox[56].setFlags(MOVED);
		if (statePart.find('q') == std::string::npos) mailbox[0].setFlags(MOVED);

		stateParts >> statePart;//Implement en passant

		stateParts >> statePart;
		halfMoveClock = std::stoi(statePart);

		possibleEpCapture = -1;
		promotedPawnSquare = -1;
		madeMoves.resetMadeMoves();
		//Implement rest of Forsyth–Edwards Notation
		updateAvailableMoves();
	}

	MadeMove getMadeMove(Move move) {
		MadeMove madeMove;
		madeMove.previousEpCapture = possibleEpCapture;
		madeMove.movedPieceFlags = mailbox[move.getFrom()].getFlags();
		madeMove.move = move;
		madeMove.halfMoveClock = this->halfMoveClock;
		if (move.isCapture()) {
			int capturedSquare;
			if (move.isEpCapture()) capturedSquare = mailbox.getCapturedEpSquare(move);
			else capturedSquare = move.getTo();
			madeMove.capturedPiece = mailbox[capturedSquare];
			
		}
		return madeMove;
	}

	int makeMove(Move move, bool waitForSelection = false, bool updateMoves = true, bool updateMadeMoves = true) {
		if (updateMadeMoves) {
			MadeMove madeMove = getMadeMove(move);
			madeMoves.addMove(madeMove);
		}

		int promotionCause = 0;
		halfMoveClock++;
		if (mailbox[move.getFrom()].getType() == PAWN) halfMoveClock = 0;
		if (move.isCapture()) {
			halfMoveClock = 0;
			int capturedSquare;
			if (move.isEpCapture()) capturedSquare = mailbox.getCapturedEpSquare(move);
			else capturedSquare = move.getTo();
			pieceList.removePiece(capturedSquare, sideToMove ^ WHITE, mailbox[capturedSquare].getType());
			mailbox[capturedSquare].setType(EMPTY);
		}

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

		pieceList.movePiece(move.getFrom(), move.getTo(), sideToMove);
		mailbox.movePiece(move.getFrom(), move.getTo());
		
		if (move.isCastle()) {
			std::pair<int, int> rookMove = mailbox.getRookMoveFromCastle(move);
			mailbox.movePiece(rookMove.first, rookMove.second);
			pieceList.movePiece(rookMove.first, rookMove.second, sideToMove);
		}
		else if (move.isPromotion()) {
			promotedPawnSquare = move.getTo();
			if (!waitForSelection)
				promotionCause = updatePromotion((move.getFlags() & (~0x4)) - 8, updateMoves);
		}
		
		if (!move.isPromotion()) { //Promotion happens first, then moves are updated
			sideToMove ^= WHITE;
			if (updateMoves) {
				updateAvailableMoves();
				return checkGameEnded();
			}
		}
		return promotionCause;
	}

	void unmakeLastMove(bool updateMoves = false) {
		if (madeMoves.nMadeMoves == 0) return;

		MadeMove lastMove = madeMoves.getLastMove();
		Move& move = lastMove.move;
		madeMoves.popLastMove();
		unmakeMove(lastMove, updateMoves);
	}

	void unmakeMove(MadeMove& lastMove, bool updateMoves = false) {
		halfMoveClock = lastMove.halfMoveClock;
		Move& move = lastMove.move;
		sideToMove ^= WHITE;
		possibleEpCapture = lastMove.previousEpCapture;
		if (possibleEpCapture != -1) mailbox[possibleEpCapture].setFlags(EN_PASSANT);

		mailbox.movePiece(move.getTo(), move.getFrom());
		mailbox[move.getFrom()].setFlags(lastMove.movedPieceFlags);
		if (move.isPromotion()) {
			pieceList.nSpecPieces[sideToMove][mailbox[move.getFrom()].getType() - 1]--;
			pieceList.nSpecPieces[sideToMove][PAWN - 1]++;
			mailbox[move.getFrom()].setType(PAWN);
		}
		pieceList.movePiece(move.getTo(), move.getFrom(), sideToMove);

		if (move.isCapture()) {
			int capturedSquare;
			if (move.isEpCapture()) capturedSquare = mailbox.getCapturedEpSquare(move);
			else capturedSquare = move.getTo();
			mailbox[capturedSquare] = lastMove.capturedPiece;
			pieceList.addPiece(capturedSquare, sideToMove ^ WHITE, lastMove.capturedPiece.getType());
		}

		if (move.isCastle()) {
			std::pair<int, int> rookMove = mailbox.getRookMoveFromCastle(move);
			mailbox.movePiece(rookMove.second, rookMove.first);
			pieceList.movePiece(rookMove.second, rookMove.first, sideToMove);
		}

		if (updateMoves)
			updateAvailableMoves();
	}

	void goForthMadeMoves() {
		if (madeMoves.maxMadeMoves == madeMoves.nMadeMoves) return;
		Move move = madeMoves.getNextMove().move;
		makeMove(move);
	}

	int updatePromotion(int promotion, bool updateMoves = true) {
		mailbox[promotedPawnSquare].setType(KNIGHT + promotion);
		madeMoves.updateLastPromotionMove(promotion);
		pieceList.nSpecPieces[sideToMove][PAWN - 1]--;
		pieceList.nSpecPieces[sideToMove][KNIGHT + promotion - 1]++;
		sideToMove ^= WHITE;
		promotedPawnSquare = -1;
		if (updateMoves) {
			updateAvailableMoves();
			return checkGameEnded();
		}
		return 0;
	}

	//Very slow but only for visuals
	Move getMove(int fromPos, int toPos) { //Promotions not implemented but should not be a problem as long as this is only used for visuals
		for (int i = 0; i < availableMoves.nMoves; i++) {
			Move& move = availableMoves[i];
			if (move.getFrom() == fromPos && move.getTo() == toPos)
				return move;
		}
		return Move();
	}
	
	void filterPseudoLegalMoves(ChessMoves& moves) {
		int nFilteredMoves = 0;
		for (int i = 0; i < moves.nMoves; i++) {
			Move& move = moves[i];

			if (move.isCastle()) {
				int squareBesidesKing;
				if (move.isQueenCastle()) squareBesidesKing = pieceList.getKingSquare(sideToMove) - 1;
				else squareBesidesKing = pieceList.getKingSquare(sideToMove) + 1;

				if (moveGenerator.squareIsAttacked(pieceList.getKingSquare(sideToMove), mailbox, sideToMove) ||
					moveGenerator.squareIsAttacked(squareBesidesKing, mailbox, sideToMove)) {

					filteredMoves[nFilteredMoves] = i;
					nFilteredMoves++;
					continue;
				}
			}
			MadeMove madeMove = getMadeMove(move);
			this->makeMove(move, false, false, false);
			// Color is reversed because makeMove inverts it
			if (moveGenerator.squareIsAttacked(pieceList.getKingSquare(sideToMove ^ WHITE), mailbox, sideToMove ^ WHITE)) {
				filteredMoves[nFilteredMoves] = i;
				nFilteredMoves++;
			}
			this->unmakeMove(madeMove);
		}
		for (int i = nFilteredMoves - 1; i >= 0; i--)
			moves.removeMove(filteredMoves[i]);
	}

	void updateAvailableMoves() {
		moveGenerator.updatePossibleMoves(mailbox, pieceList, availableMoves, sideToMove);
		this->filterPseudoLegalMoves(availableMoves);
	}

	int checkGameEnded() {
		std::cout << halfMoveClock << "\n";
		if (availableMoves.nMoves == 0) {
			if (!moveGenerator.squareIsAttacked(pieceList.kingSquare[sideToMove], mailbox, sideToMove))
				return STALEMATE;
			else
				return CHECKMATE;
		}
		if (pieceList.nPieces[WHITE] <= 3 && pieceList.nPieces[BLACK] <= 3 && 
			(pieceList.nPieces[WHITE] == 1 || 
			(pieceList.nPieces[WHITE] == 2 && (pieceList.nSpecPieces[WHITE][BISHOP - 1] == 1 || pieceList.nSpecPieces[WHITE][KNIGHT - 1] == 1)) ||
			(pieceList.nPieces[WHITE] == 3 && pieceList.nSpecPieces[WHITE][KNIGHT - 1] == 2))
			&&
			(pieceList.nPieces[BLACK] == 1 ||
			(pieceList.nPieces[BLACK] == 2 && (pieceList.nSpecPieces[BLACK][BISHOP - 1] == 1 || pieceList.nSpecPieces[BLACK][KNIGHT - 1] == 1)) ||
			(pieceList.nPieces[BLACK] == 3 && pieceList.nSpecPieces[BLACK][KNIGHT - 1] == 2)))
			return INSUFFICIENT_MATERIAL;
		if (halfMoveClock == 100)
			return MOVE_RULE;

		return 0;
	}
};

#endif
