#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include "chessPiece.h"
#include "mailbox.h"
#include "pieceList.h"
#include "chessMove.h"

class MoveGenerator {

	int pieceOffset[6][8] = {
		{   0,   0,  0,  0, 0,  0,  0,  0 }, // EMPTY
		{ -21, -19,-12, -8, 8, 12, 19, 21 }, // KNIGHT
		{ -11,  -9,  9, 11, 0,  0,  0,  0 }, // BISHOP
		{ -10,  -1,  1, 10, 0,  0,  0,  0 }, // ROOK
		{ -11, -10, -9, -1, 1,  9, 10, 11 }, // QUEEN
		{ -11, -10, -9, -1, 1,  9, 10, 11 }  // KING
	};
	int pieceOffsets[6] = { 0, 8, 4, 4, 8, 8 };
	bool slidingPiece[6] = { false, false, true, true, true, false };
	
public:

	void updatePossibleMoves(Mailbox& mailbox, PieceList& pieceList, ChessMoves& moves, int sideToMove) {

		moves.resetMoves();
		int (&piecePositions)[16] = (sideToMove ? pieceList.whitePieces : pieceList.blackPieces);
		
		for (int i = 0; i < 16; i++) {
			int& fromSquare = piecePositions[i];
			Piece& piece = mailbox[fromSquare];

			if (piece.getType() != PAWN) {
				//All moves but pawns and castling
				for (int j = 0; j < pieceOffsets[piece.getType() - 1]; j++) {
					for (int toSquare = fromSquare;;) {
						toSquare = mailbox.getSquareWithOffset(toSquare, pieceOffset[piece.getType() - 1][j]);
						if (toSquare == -1)
							break;

						Piece& capturedPiece = mailbox[toSquare];
						if (capturedPiece.getType() != EMPTY) {
							if (capturedPiece.getColor() != sideToMove) 
								moves.addMove(fromSquare, toSquare, CAPTURE);
							break;
						}

						moves.addMove(fromSquare, toSquare, QUIET_MOVE);
						if (!slidingPiece[piece.getType() - 1])
							break;
					}
				}
				//Castling
				if (piece.getType() == KING && !piece.hasMoved()) {
					int row;
					if (sideToMove == WHITE) row = 7;
					else row = 0;

					Piece& kingRook = mailbox[row * 8 + 7];
					Piece& queenRook = mailbox[row * 8];
					bool queenSideEmpty = true, kingSideEmpty = true;

					//****************************************************************************
					// CANNOT CASTLE IF KING IS IN CHECK
					//****************************************************************************

					if (queenRook.getType() == ROOK && !queenRook.hasMoved()) {
						for (int j = 0; j < 3 && queenSideEmpty; j++)
							if (mailbox[row * 8 + 1 + j].getType() != EMPTY)
								queenSideEmpty = false;
						if (queenSideEmpty)
							moves.addMove(fromSquare, row * 8 + 2, QUEEN_CASTLE);
					}

					if (kingRook.getType() == ROOK && !kingRook.hasMoved()) {
						for (int j = 0; j < 2 && kingSideEmpty; j++)
							if (mailbox[row * 8 + 5 + j].getType() != EMPTY)
								kingSideEmpty = false;
						if (kingSideEmpty)
							moves.addMove(fromSquare, row * 8 + 6, KING_CASTLE);
					}
				}
				
			}
			//Pawn
			else {
				int forwardOffset;
				if (sideToMove == WHITE) forwardOffset = -8;
				else forwardOffset = 8;

				bool squareBeforePromotion = false;
				if ((sideToMove == WHITE && mailbox.getRow(fromSquare) == 1) ||
					sideToMove == BLACK && mailbox.getRow(fromSquare) == 6)
					squareBeforePromotion = true;

				if (mailbox[fromSquare + forwardOffset].getType() == EMPTY) {
					if (squareBeforePromotion) {
						moves.addMove(fromSquare, fromSquare + forwardOffset, KNIGHT_PROMOTION);
						moves.addMove(fromSquare, fromSquare + forwardOffset, BISHOP_PROMOTION);
						moves.addMove(fromSquare, fromSquare + forwardOffset, ROOK_PROMOTION);
						moves.addMove(fromSquare, fromSquare + forwardOffset, QUEEN_PROMOTION);
					}
					else {
						moves.addMove(fromSquare, fromSquare + forwardOffset, QUIET_MOVE);
						if (!piece.hasMoved() && mailbox[fromSquare + 2 * forwardOffset].getType() == EMPTY)
							moves.addMove(fromSquare, fromSquare + 2 * forwardOffset, DOUBLE_PAWN_PUSH);
					}
				}

				addPawnCaptures(moves, pieceList, mailbox, fromSquare, forwardOffset,  1, sideToMove, squareBeforePromotion);
				addPawnCaptures(moves, pieceList, mailbox, fromSquare, forwardOffset, -1, sideToMove, squareBeforePromotion);
			}
		}
	}


private:

	void addPawnCaptures(ChessMoves& moves, PieceList& pieceList, Mailbox& mailbox, 
		int fromSquare, int forwardOffset, int sideOffset, int sideToMove, bool squareBeforePromotion) {

		forwardOffset < 0 ? forwardOffset -= 2 : forwardOffset += 2; //Because mailbox120 has different offsets
		int sideSquare = mailbox.getSquareWithOffset(fromSquare, sideOffset);
		if (sideSquare != -1 && mailbox[sideSquare].getType() == PAWN && mailbox[sideSquare].canBeCapturedEP()) {
			moves.addMove(fromSquare, mailbox.getSquareWithOffset(sideSquare, forwardOffset), EP_CAPTURE);
		}
		else {
			int diagonalSquare = mailbox.getSquareWithOffset(fromSquare, forwardOffset + sideOffset);
			if (diagonalSquare != -1) {
				Piece& capturePiece = mailbox[diagonalSquare];
				if (capturePiece.getType() != EMPTY && capturePiece.getColor() != sideToMove) {
					if (squareBeforePromotion) {
						moves.addMove(fromSquare, diagonalSquare, KNIGHT_PROMOTION_CAP);
						moves.addMove(fromSquare, diagonalSquare, BISHOP_PROMOTION_CAP);
						moves.addMove(fromSquare, diagonalSquare, ROOK_PROMOTION_CAP);
						moves.addMove(fromSquare, diagonalSquare, QUEEN_PROMOTION_CAP);
					}
					else
						moves.addMove(fromSquare, diagonalSquare, CAPTURE);
				}
			}
		}
	}
};

#endif