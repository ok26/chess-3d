#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <map>
#include <string>
#include <algorithm>
#include <chrono>
#include <thread>

#include "model/animation.h"
#include "model/board.h"
#include "model/world_object.h"
#include "model/name_store.h"
#include "model/item_menu.h"

#include "index_model/board.h"
#include "index_model/move.h"

#include "util/camera.h"
#include "util/shader.h"
#include "util/model.h"
#include "util/read_files.h"
#include "util/text_render.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void configureShader(Shader& shader, glm::mat4& projection, glm::mat4& view);
void processMenuEvent(GLFWwindow* window);
void updateGameEnding(int gameEnding);

int SCR_WIDTH;
int SCR_HEIGHT;
const unsigned int FPS = 60;

Camera camera(glm::vec3(0.0f, 15.5f, 6.7f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0, -67.0);
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
bool freeCameraFlight = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::map<int, bool> pressedKeys;
std::array<int, 3> keysUsedInProgram = { GLFW_KEY_C };

ChessNameStore chessNameStore;
ChessBoardModel chessModel;
ChessBoardIndex chessIndex;

ItemMenu rightButtonMenu, leftButtonMenu;
int evaluationTextID, lastMoveTextID, sideToMoveTextID, depthTextID, moveTextID, blackTextButtonID, 
    whiteTextButtonID, moveTextButtonID, quitButtonID, resetBoardID, flipBoardID, goBackMoveID, goForthMoveID;
TextRenderer textRenderer;

const char* sideToMoveText[2] = { "Black to Move", "White to Move" };
const char* gameEndings[5] = { "Checkmate, ", "Stalemate, Draw", "Insuffiecient Material, Draw", "3-Fold Repetition, Draw", "50-Move Rule, Draw" };

std::array<glm::vec3, 4> pointLightPositions = {
    glm::vec3(6.0f,  2.0f,  6.0f),
    glm::vec3(-6.0f,  2.0f,  6.0f),
    glm::vec3(6.0f,  2.0f, -6.0f),
    glm::vec3(-6.0f,  2.0f, -6.0f)
};

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (primaryMonitor == nullptr) {
        std::cerr << "Failed to get primary monitor" << std::endl;
        glfwTerminate();
        return -1;
    }

    int xpos, ypos;
    glfwGetMonitorWorkarea(primaryMonitor, &xpos, &ypos, &SCR_WIDTH, &SCR_HEIGHT);
    SCR_WIDTH *= 0.95;
    SCR_HEIGHT *= 0.95;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Chess 3D", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader standardLightShader("src/shaders/normal_light.vs", "src/shaders/normal_light.fs");
    Shader menuShader("src/shaders/simple_color.vs", "src/shaders/simple_color.fs");
    Shader textShader("src/shaders/text.vs", "src/shaders/text.fs");

    std::vector<float> woodPlaneVertices = loadVertexData("src/resources/vertexData/woodPlane.txt");
    std::vector<float> chessBoardVertices = loadVertexData("src/resources/vertexData/chessBoard.txt");

    unsigned int woodPlaneVAO = initializeVertexArray(woodPlaneVertices);
    unsigned int chessBoardVAO = initializeVertexArray(chessBoardVertices);

    unsigned int woodPlaneTexture = loadTexture("src/resources/textures/woodFloor.png");
    WorldObject woodPlane(woodPlaneVAO, (int)woodPlaneVertices.size() / 8, woodPlaneTexture);

    chessIndex.changeBoardState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    chessModel.initialize(chessBoardVAO, SCR_WIDTH, SCR_HEIGHT);
    chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);

    leftButtonMenu = ItemMenu(3, 6, glm::vec3(-5.3f, 6.0f, 2.0f), -15.0f, SCR_WIDTH, SCR_HEIGHT);
    sideToMoveTextID =  leftButtonMenu.addItem(TEXT, 0.0f, -0.7f, 2.8f, 0.5f, sideToMoveText[WHITE], false, ORANGE, 0.9f);
    evaluationTextID =  leftButtonMenu.addItem(TEXT, 0.0f, -0.5f, 2.8f, 0.5f, "Evaluation: -0.25", false, RED, 1.0f);
    depthTextID =       leftButtonMenu.addItem(TEXT, 0.0f, -0.35f, 2.8f, 0.5f, "Depth: 7", false, LIGHT_GREY, 0.3f);
    moveTextID =        leftButtonMenu.addItem(TEXT, 0.0f, -0.2f, 2.8f, 0.5f, "Move: --", false, PURPLE, 0.9f);
                        leftButtonMenu.addItem(TEXT, 0.0f, 0.25f, 2.8f, 0.5f, "AI Plays as:", true, GREY, 0.9f);
    whiteTextButtonID = leftButtonMenu.addItem(TEXT_BUTTON, -0.46f, 0.42f, 1.2f, 0.5f, "White", true, LIGHT_GREY, 1.0f);
    blackTextButtonID = leftButtonMenu.addItem(TEXT_BUTTON, 0.46f, 0.42f, 1.2f, 0.5f, "Black", true, LIGHT_GREY, 1.0f);
    quitButtonID =      leftButtonMenu.addItem(TEXT_BUTTON, 0.0f, 0.75f, 1.5f, 0.5f, "Quit", true, LIGHT_GREY, 1.0f);

    rightButtonMenu = ItemMenu(3, 6, glm::vec3(5.3f, 6.0f, 2.0f), 15.0f, SCR_WIDTH, SCR_HEIGHT);
    lastMoveTextID =    rightButtonMenu.addItem(TEXT, 0.1f, -0.6f, 2.8f, 0.5f, "Last Move: ---", false, GREY, 0.9f);
    goBackMoveID =      rightButtonMenu.addItem(ICON_BUTTON, -0.35f, -0.35f, 0.5f, 0.5f, "src/resources/textures/leftArrow.png", true, LIGHT_GREY, 1.0f);
    goForthMoveID =     rightButtonMenu.addItem(ICON_BUTTON, 0.35f, -0.35f, 0.5f, 0.5f, "src/resources/textures/rightArrow.png", true, LIGHT_GREY, 1.0f);
    moveTextButtonID =  rightButtonMenu.addItem(TEXT_BUTTON, 0.0f, 0.25f, 1.5f, 0.5f, "Show Best Move", true, LIGHT_GREY, 1.0f);
    flipBoardID =       rightButtonMenu.addItem(TEXT_BUTTON, 0.0f, 0.45f, 1.5f, 0.5f, "Flip Board", true, LIGHT_GREY, 1.0f);
    resetBoardID =      rightButtonMenu.addItem(TEXT_BUTTON, 0.0f, 0.75f, 1.5f, 0.5f, "Reset Board", true, LIGHT_GREY, 1.0f);

    textRenderer.initializeFont("src/resources/fonts/Antonio-Regular.ttf");

    standardLightShader.use();
    standardLightShader.setInt("material.diffuse", 0);

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::max((int)((1.0f / FPS - deltaTime) * 1000), 0)));
       
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        standardLightShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        configureShader(standardLightShader, projection, view);
        
        glm::mat4 model(1.0f);
        standardLightShader.setMat4("model", model);
        woodPlane.draw();
        chessModel.draw(standardLightShader, projection, view, lastX, lastY);

        leftButtonMenu.draw(menuShader, textShader, projection, view, textRenderer, lastX, lastY, freeCameraFlight);
        rightButtonMenu.draw(menuShader, textShader, projection, view, textRenderer, lastX, lastY, freeCameraFlight);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}


void configureShader(Shader& shader, glm::mat4& projection, glm::mat4& view) {
    
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("viewPos", camera.Position);

    float dirLightZ = sin((float)glfwGetTime() / 5);
    float dirLightX = cos((float)glfwGetTime() / 5);

    shader.setVec3("dirLight.direction", -dirLightX, -1.0f, dirLightZ);
    shader.setVec3("dirLight.ambient", 0.15f, 0.15f, 0.15f);
    shader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
    shader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);

    for (int i = 0; i < 4; i++) {
        shader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
        shader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.15f, 0.15f, 0.15f);
        shader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.3f, 0.3f, 0.3f);
        shader.setVec3("pointLights[" + std::to_string(i) + "].specular", 0.8f, 0.8f, 0.8f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.07f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.017f);
    }

    shader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("material.shininess", 64.0f);
}


void processInput(GLFWwindow* window) {

    if (freeCameraFlight) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !pressedKeys[GLFW_KEY_C]) {
        pressedKeys[GLFW_KEY_C] = true;
        freeCameraFlight ^= 1;
        glfwSetInputMode(window, GLFW_CURSOR, freeCameraFlight ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    for (int& key : keysUsedInProgram)
        if (glfwGetKey(window, key) == GLFW_RELEASE)
            pressedKeys[key] = false;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (freeCameraFlight)
        camera.ProcessMouseMovement(xoffset, yoffset);
    
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (freeCameraFlight)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        int clickedSquare = chessModel.getClickedSquare((int)lastX, (int)lastY, projection, view);

        std::pair<int, int> moveInfo;
        if (action == GLFW_PRESS) {
            moveInfo = chessModel.processMouseClick(clickedSquare);
            if (rightButtonMenu.getHoveredItemID() != -1 || leftButtonMenu.getHoveredItemID() != -1)
                processMenuEvent(window);
        }
        else 
            moveInfo = chessModel.processMouseRelease(clickedSquare);

        if (moveInfo.first != -1) {
            Move move = chessIndex.getMove(moveInfo.first, moveInfo.second);
            int gameEnding = chessIndex.makeMove(move, true);
            if (gameEnding) updateGameEnding(gameEnding);
            else leftButtonMenu.updateItemText(sideToMoveTextID, sideToMoveText[chessIndex.sideToMove]);

            chessIndex.madeMoves.maxMadeMoves = chessIndex.madeMoves.nMadeMoves;
            if (chessIndex.promotedPawnSquare != -1)
                chessModel.isPromotingPawn = true;

            chessModel.doMove(move, chessIndex.mailbox);
            chessModel.updateAvailableMoves(chessIndex.availableMoves);
        } 
        else if (moveInfo.second != -1) { //Pawn promotion has been selected
            chessModel.updatePromotion(chessIndex.promotedPawnSquare, moveInfo.second);
            int gameEnding = chessIndex.updatePromotion(moveInfo.second);
            if (gameEnding) updateGameEnding(gameEnding);
            chessModel.updateAvailableMoves(chessIndex.availableMoves);
        }
    }
}

void updateGameEnding(int gameEnding) {
    std::string gameEndingDesc = gameEndings[gameEnding - 1];
    if (gameEnding == CHECKMATE) {
        if (chessIndex.sideToMove == WHITE) gameEndingDesc += "Black wins";
        else gameEndingDesc += "White wins";
    }
    leftButtonMenu.updateItemText(sideToMoveTextID, gameEndingDesc);
    chessIndex.availableMoves.resetMoves();
}

void processMenuEvent(GLFWwindow* window) {
    if (rightButtonMenu.getHoveredItemID() != -1) {
        int ID = rightButtonMenu.getHoveredItemID();

        if (ID == moveTextButtonID) {
            rightButtonMenu.invertTextButtonColor(ID, GREEN, LIGHT_GREY);
        }
        else if (ID == goBackMoveID) {
            chessIndex.unmakeLastMove(true);
            leftButtonMenu.updateItemText(sideToMoveTextID, sideToMoveText[chessIndex.sideToMove]);
            chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);
        }
        else if (ID == goForthMoveID) {
            chessIndex.goForthMadeMoves();
            leftButtonMenu.updateItemText(sideToMoveTextID, sideToMoveText[chessIndex.sideToMove]);
            chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);
        }
        else if (ID == flipBoardID) {
            chessModel.doFlipBoardAnimation();
        }
        else if (ID == resetBoardID) {
            leftButtonMenu.updateItemText(sideToMoveTextID, sideToMoveText[WHITE]);
            chessIndex.changeBoardState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);
        }
    }
    else if (leftButtonMenu.getHoveredItemID() != -1) {
        int ID = leftButtonMenu.getHoveredItemID();

        if (ID == blackTextButtonID) {
            leftButtonMenu.invertTextButtonColor(ID, GREEN, LIGHT_GREY);
        }
        else if (ID == whiteTextButtonID) {
            leftButtonMenu.invertTextButtonColor(ID, GREEN, LIGHT_GREY);
        }
        else if (ID == quitButtonID)
            glfwSetWindowShouldClose(window, true);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //camera.ProcessMouseScroll(static_cast<float>(yoffset));
}