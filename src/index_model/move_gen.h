#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include "index_model/piece.h"
#include "index_model/mailbox.h"
#include "index_model/piece_list.h"
#include "index_model/move.h"

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

	int filteredMoves[MAX_AVAILABLE_MOVES];
	
public:

	void updatePossibleMoves(Mailbox& mailbox, PieceList& pieceList, ChessMoves& moves, int sideToMove) {

		moves.resetMoves();
		int (&piecePositions)[16] = pieceList.pieces[sideToMove];
		int& nPieces = pieceList.nPieces[sideToMove];

		for (int i = 0; i < nPieces; i++) {
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

				bool squareBeforePromotion = ((sideToMove == WHITE && mailbox.getRow(fromSquare) == 1) ||
					sideToMove == BLACK && mailbox.getRow(fromSquare) == 6);
				bool inStartingSquare = ((sideToMove == BLACK && mailbox.getRow(fromSquare) == 1) ||
					sideToMove == WHITE && mailbox.getRow(fromSquare) == 6);

				if (mailbox[fromSquare + forwardOffset].getType() == EMPTY) {
					if (squareBeforePromotion) {
						moves.addMove(fromSquare, fromSquare + forwardOffset, KNIGHT_PROMOTION);
						moves.addMove(fromSquare, fromSquare + forwardOffset, BISHOP_PROMOTION);
						moves.addMove(fromSquare, fromSquare + forwardOffset, ROOK_PROMOTION);
						moves.addMove(fromSquare, fromSquare + forwardOffset, QUEEN_PROMOTION);
					}
					else {
						moves.addMove(fromSquare, fromSquare + forwardOffset, QUIET_MOVE);
						if (inStartingSquare && mailbox[fromSquare + 2 * forwardOffset].getType() == EMPTY)
							moves.addMove(fromSquare, fromSquare + 2 * forwardOffset, DOUBLE_PAWN_PUSH);
					}
				}

				addPawnCaptures(moves, pieceList, mailbox, fromSquare, forwardOffset,  1, sideToMove, squareBeforePromotion);
				addPawnCaptures(moves, pieceList, mailbox, fromSquare, forwardOffset, -1, sideToMove, squareBeforePromotion);
			}
		}
	}

	bool squareIsAttacked(int square, Mailbox& mailbox, int color) {
		for (int piece = KNIGHT; piece <= KING; piece++) {
			for (int i = 0; i < pieceOffsets[piece - 1]; i++) {
				for (int toSquare = square;;) {
					toSquare = mailbox.getSquareWithOffset(toSquare, pieceOffset[piece - 1][i]);
					if (toSquare == -1)
						break;

					Piece& attackingPiece = mailbox[toSquare];
					if (attackingPiece.getType() != EMPTY) {
						if (attackingPiece.getColor() != color && attackingPiece.getType() == piece)
							return true;
						break;
					}
					if (!slidingPiece[piece - 1])
						break;
				}
			}
		}
		int opposingPawnOffset;
		if (color == WHITE) opposingPawnOffset = -10;
		else opposingPawnOffset = 10;
		Piece& possibleOppPawn1 = mailbox[mailbox.getSquareWithOffset(square, opposingPawnOffset + 1)];
		Piece& possibleOppPawn2 = mailbox[mailbox.getSquareWithOffset(square, opposingPawnOffset - 1)];
		if (possibleOppPawn1.getType() == PAWN && possibleOppPawn1.getColor() != color) return true;
		if (possibleOppPawn2.getType() == PAWN && possibleOppPawn2.getColor() != color) return true;
		
		return false;
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