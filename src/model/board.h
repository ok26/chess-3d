#ifndef CHESS_BOARD_MODEL_H
#define CHESS_BOARD_MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <bitset>

#include "model/piece.h"
#include "model/world_object.h"
#include "model/animation.h"

#include "index_model/mailbox.h"
#include "index_model/move.h"

#include "util/model.h"
#include "util/shader.h"
#include "util/read_files.h"
#include "util/convert_coords.h"

const float pieceYPosition = 1.0f;
const float moveAnimationDuration = 0.25f;

class ChessBoardModel {

    int scr_width, scr_height;

	std::vector<Model> pieceModels;
    
    unsigned int boardVAO;

    unsigned int pieceTextures[2];
    unsigned int boardPlaneTexture;
    unsigned int boardFacadeTexture;

    WorldObject selectedSquareIndicator1, selectedSquareIndicator2, hoveredPawnPromotionIndicator;
    Shader selectedSquareIndicatorShader;

    bool animateMove = false;
    
    int pieceHoveredDuringPawnPromotion = -1;
    Animation flipBoardAnimation;
    bool isFlipped = false;

public:
    PieceModel piecesOnBoard[64];
    std::bitset<64> availableMoves[64];
    int sideToMove;
    int selectedPieceSquare = -1;
    bool isPromotingPawn = false;

    void initialize(unsigned int boardVAO, int scr_width, int scr_height) {
        for (int i = 0; i < 64; i++)
            piecesOnBoard[i].initialize(glm::vec3(-3.5 + i % 8, pieceYPosition, -3.5 + i / 8), 
                                        glm::vec3(-3.5 + (7 - (i % 8)), pieceYPosition, -3.5 + (7 - i / 8)), -1);

        this->boardVAO = boardVAO;
        this->scr_height = scr_height;
        this->scr_width = scr_width;

        pieceModels.push_back(Model("src/resources/models/pawn/Pawn.obj"));
        pieceModels.push_back(Model("src/resources/models/knight/Knight.obj"));
        pieceModels.push_back(Model("src/resources/models/bishop/Bishop.obj"));
        pieceModels.push_back(Model("src/resources/models/rook/Rook.obj"));
        pieceModels.push_back(Model("src/resources/models/queen/Queen.obj"));
        pieceModels.push_back(Model("src/resources/models/king/King.obj"));

        boardPlaneTexture = loadTexture("src/resources/textures/chessBoardPlane.png");
        boardFacadeTexture = loadTexture("src/resources/textures/chessBoardFacade.png");

        pieceTextures[0] = loadTexture("src/resources/textures/blackPiece.png");
        pieceTextures[1] = loadTexture("src/resources/textures/whitePiece.png");

        std::vector<float> squareVertexData = loadVertexData("src/resources/vertexData/square.txt");
        unsigned int squareVAO = initializeVertexArray(squareVertexData);
        unsigned int selectedSquareTexture1 = loadTexture("src/resources/textures/circle.png");
        unsigned int selectedSquareTexture2 = loadTexture("src/resources/textures/hollowCircle.png");
        unsigned int fullCircleTexture = loadTexture("src/resources/textures/fullCircle.png");
        selectedSquareIndicator1 = WorldObject(squareVAO, (int)squareVertexData.size() / 8, selectedSquareTexture1);
        selectedSquareIndicator2 = WorldObject(squareVAO, (int)squareVertexData.size() / 8, selectedSquareTexture2);
        hoveredPawnPromotionIndicator = WorldObject(squareVAO, (int)squareVertexData.size() / 8, fullCircleTexture);
        selectedSquareIndicatorShader = Shader("src/shaders/simpleColorShader.vs", "src/shaders/simpleColorShader.fs");
        selectedSquareIndicatorShader.use();
        selectedSquareIndicatorShader.setInt("diffuseMap", 0);
    }

    void updateGameData(Mailbox& mailbox, ChessMoves& generatedMoves, bool sideToMove) {
        for (int i = 0; i < 64; i++) {
            if (mailbox[i].getType() != EMPTY) {

                piecesOnBoard[i].type = mailbox[i].getType();
                piecesOnBoard[i].color = mailbox[i].getColor();
                piecesOnBoard[i].drawn = true;
            }
            else if (piecesOnBoard[i].drawn)
                piecesOnBoard[i].drawn = false;
        }
        this->sideToMove = sideToMove;
        selectedPieceSquare = -1;
        isPromotingPawn = false;

        updateAvailableMoves(generatedMoves);
    }

    void updateAvailableMoves(ChessMoves& generatedMoves) {
        for (int i = 0; i < 64; i++)
            availableMoves[i].reset();
        for (int i = 0; i < generatedMoves.nMoves; i++) {
            availableMoves[generatedMoves[i].getFrom()][generatedMoves[i].getTo()] = 1;
        }
    }

    int getClickedSquare(int clickedScreenX, int clickedScreenY, glm::mat4& projection, glm::mat4& view) {
        
        glm::vec3 objCoord = convert2DCoordTo3D(clickedScreenX, clickedScreenY, scr_width, scr_height, 14.5f, pieceYPosition, projection, view);
        int sqX = (int)(objCoord.x + 4.0f);
        int sqZ = (int)(objCoord.z + 4.0f);

        if (objCoord.x >= -4.0f && objCoord.x < 4.0f && objCoord.z >= -4.0f && objCoord.z < 4.0f) {
            if (isFlipped) return (7 - sqZ) * 8 + (7 - sqX);
            else return sqZ * 8 + sqX;
        }
        else
            return -1;
        
    }

    std::pair<int, int> processMouseClick(int square) {
        // If a piece is currently being animated
        if (selectedPieceSquare != -1 && piecesOnBoard[selectedPieceSquare].animation.running)
            return { -1, -1 };

        //Pawn promotion menu
        if (isPromotingPawn) {
            return { -1, pieceHoveredDuringPawnPromotion };
        }
        else {
            // If mouse clicked outside of board
            if (square == -1) {
                selectedPieceSquare = -1;
                return { -1, -1 };
            }

            // If piece of same color as already selected piece is clicked
            if (piecesOnBoard[square].drawn && (piecesOnBoard[square].color == sideToMove) &&
                (selectedPieceSquare == -1 || !availableMoves[selectedPieceSquare][square])) {

                selectedPieceSquare = square;
                piecesOnBoard[square].followingMouse = true;
                return { -1, -1 };
            }

            // If empty square is clicked while nothing is selected
            if (selectedPieceSquare == -1)
                return { -1, -1 };

            // If none of the above and the requested move is possible, animate a move to given square
            if (availableMoves[selectedPieceSquare][square]) {
                animateMove = true;
                return { selectedPieceSquare, square };
            }

            //Empty square clicked
            selectedPieceSquare = -1;
            return { -1, -1 };
        }
    }

    std::pair<int, int> processMouseRelease(int square) {
        if (selectedPieceSquare == -1 || piecesOnBoard[selectedPieceSquare].animation.running)
            return { -1, -1 };

        piecesOnBoard[selectedPieceSquare].followingMouse = false;

        if (square != -1 && availableMoves[selectedPieceSquare][square]) {
            animateMove = false;
            return { selectedPieceSquare, square };
        }
        return { -1, -1 };
    }
    
    void doMove(Move move, Mailbox& mailbox) {

        if (move.isCastle()) {
            std::pair<int, int> rookMove = mailbox.getRookMoveFromCastle(move);
            if (animateMove) {
                PieceModel& rookPiece = piecesOnBoard[rookMove.first];
                rookPiece.movingToSquare = rookMove.second;
                rookPiece.animation.doAnimation(moveAnimationDuration, (float)glfwGetTime());
                rookPiece.animationTargetPos = piecesOnBoard[rookMove.second].position;
            }
            else
                finishMove(rookMove.first, rookMove.second);
        }
        else if (move.isEpCapture())
            piecesOnBoard[mailbox.getCapturedEpSquare(move)].drawn = false;


        PieceModel& selectedPiece = piecesOnBoard[move.getFrom()];
        if (animateMove) {
            selectedPiece.movingToSquare = move.getTo();
            selectedPiece.animation.doAnimation(moveAnimationDuration, (float)glfwGetTime());
            selectedPiece.animationTargetPos = piecesOnBoard[move.getTo()].position;
        }
        else {
            finishMove(move.getFrom(), move.getTo());
            selectedPiece.movingToSquare = -1;
            selectedPieceSquare = -1;
        }
        if (!isPromotingPawn)
            sideToMove ^= WHITE;
    }
    
    void finishMove(int fromPos, int toPos) {
        piecesOnBoard[fromPos].drawn = false;
        piecesOnBoard[toPos].drawn = true;
        std::swap(piecesOnBoard[toPos].type, piecesOnBoard[fromPos].type);
        std::swap(piecesOnBoard[toPos].color, piecesOnBoard[fromPos].color);
    }

    void updatePromotion(int square, int promotion) {
        piecesOnBoard[square].type = KNIGHT + promotion;
        sideToMove ^= WHITE;
        isPromotingPawn = false;
    }
    
	void draw(Shader& shader, glm::mat4& projection, glm::mat4& view, int mouseX, int mouseY) {

        float animationRotation = 0.0f;
        if (flipBoardAnimation.running) {
            flipBoardAnimation.updateAnimation(glfwGetTime());
            if (flipBoardAnimation.finished)
                isFlipped ^= 1;
            else
                animationRotation = 180.0f * flipBoardAnimation.animationStep;
        }

        //Chess board
        glBindVertexArray(boardVAO);

        glm::mat4 boardModel(1.0f);
        boardModel = glm::rotate(boardModel, glm::radians(270.0f + 180.0f * isFlipped + animationRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", boardModel);

        glBindTexture(GL_TEXTURE_2D, boardPlaneTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, boardFacadeTexture);
        glDrawArrays(GL_TRIANGLES, 6, 96);

        //Indicators for available moves of currently selected piece
        if (selectedPieceSquare != -1 && !piecesOnBoard[selectedPieceSquare].animation.running) {
            selectedSquareIndicatorShader.use();
            selectedSquareIndicatorShader.setMat4("projection", projection);
            selectedSquareIndicatorShader.setMat4("view", view);
            selectedSquareIndicatorShader.setVec3("color", glm::vec3(0.1f, 0.4f, 1.0f));
            selectedSquareIndicatorShader.setFloat("alpha", 0.3f);
            for (int i = 0; i < 64; i++)
                if (availableMoves[selectedPieceSquare][i]) {

                    glm::mat4 model(1.0f);
                    model = glm::rotate(model, glm::radians(180.0f * isFlipped + animationRotation), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::translate(model, piecesOnBoard[i].position + glm::vec3(0.0f, 0.02f, 0.0f));
                    selectedSquareIndicatorShader.setMat4("model", model);
                    if (piecesOnBoard[i].drawn) 
                        selectedSquareIndicator2.draw();
                    else 
                        selectedSquareIndicator1.draw();
                }
        }
   
        //All pieces currently on the board
        for (int square = 0; square < 64; square++) {
            PieceModel& piece = piecesOnBoard[square];
            if (piece.drawn) {
                Model& pieceModel = pieceModels[piece.type - 1];
                unsigned int& texture = pieceTextures[piece.color];

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::radians(180.0f * isFlipped + animationRotation), glm::vec3(0.0f, 1.0f, 0.0f));

                if (piece.followingMouse) {
                    glm::vec3 mouse3DPos = convert2DCoordTo3D(mouseX, mouseY, scr_width, scr_height, 14.5f, pieceYPosition, projection, view);
                    piece.followPosition(model, mouse3DPos, isFlipped);
                }
                else if (piece.movingToSquare != -1) {
                    piece.moveTowardsTargetedPosition(model);
                    
                    piece.animation.updateAnimation((float)glfwGetTime());
                    if (piece.animation.finished) {
                        finishMove(square, piece.movingToSquare);
                        piece.movingToSquare = -1;
                        selectedPieceSquare = -1;
                    }   
                }
                    
                model = glm::translate(model, piece.position);

                if (piece.color == WHITE)
                    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));

                shader.use();
                shader.setMat4("model", model);

                glBindTexture(GL_TEXTURE_2D, texture);
                pieceModel.Draw(shader);
            }
        }

        //Menu to select what piece promoted pawn will turn into
        if (isPromotingPawn) {
            for (int i = 0; i < 4; i++) {
                glm::mat4 model(1.0f);
                model = glm::translate(model, glm::vec3(-3.0f + 2.0f * i, 5.0f, 2.5f));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians((float)glfwGetTime() * 45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));

                shader.use();
                shader.setMat4("model", model);
                unsigned int texture = pieceTextures[sideToMove];
                Model pieceModel = pieceModels[KNIGHT + i - 1];
                glBindTexture(GL_TEXTURE_2D, texture);
                pieceModel.Draw(shader);
            }

            if (mouseY < 600 && mouseY > 375 && mouseX >= 450 && mouseX < 1450) { //Hardcoded because game always in fullscreen
                pieceHoveredDuringPawnPromotion = (mouseX - 450) / 250;
                glm::mat4 model(1.0f);
                model = glm::translate(model, glm::vec3(-3.0f + 2.0f * pieceHoveredDuringPawnPromotion, 5.0f, 1.8f));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::scale(model, glm::vec3(2.45f, 2.45f, 2.45f));
                selectedSquareIndicatorShader.use();
                selectedSquareIndicatorShader.setMat4("projection", projection);
                selectedSquareIndicatorShader.setMat4("view", view);
                selectedSquareIndicatorShader.setVec3("color", glm::vec3(0.1f, 0.4f, 1.0f));
                selectedSquareIndicatorShader.setFloat("alpha", 0.3f);
                selectedSquareIndicatorShader.setMat4("model", model);
                hoveredPawnPromotionIndicator.draw();
            }
            else
                pieceHoveredDuringPawnPromotion = -1;
        }
	}

    void doFlipBoardAnimation() {
        if (flipBoardAnimation.running) return;
        flipBoardAnimation.doAnimation(1.0f, glfwGetTime());
    }
};

#endif
