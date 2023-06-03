#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model/animation.h"
#include "Model/chessBoardModel.h"
#include "Model/worldObject.h"
#include "Model/nameStorage.h"

#include "IndexModel/chessBoardIndex.h"
#include "IndexModel/chessMove.h"

#include "Util/camera.h"
#include "Util/shader.h"
#include "Util/model.h"
#include "Util/readFiles.h"

#include <array>
#include <map>
#include <algorithm>
#include <chrono>
#include <thread>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void configureShader(Shader& shader, glm::mat4& projection, glm::mat4& view);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
const unsigned int FPS = 60;

Camera camera(glm::vec3(0.0f, 15.5f, 6.7f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0, -67.0);
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::map<int, bool> pressedKeys;
std::array<int, 2> keysUsedInProgram = { GLFW_KEY_ENTER, GLFW_KEY_T };

ChessNameStore chessNameStore;
ChessBoardModel chessModel;
ChessBoardIndex chessIndex;

std::array<WorldObject, 4> pointLights;
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

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Chess AI 3D", glfwGetPrimaryMonitor(), NULL);
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

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader standardLightShader("resources/shaders/normalLightShader.vs", "resources/shaders/normalLightShader.fs");
    Shader lightObjShader("resources/shaders/lightObjShader.vs", "resources/shaders/lightObjShader.fs");

    std::vector<float> woodPlaneVertices = loadVertexData("resources/vertexData/woodPlane.txt");
    std::vector<float> lightCubeVertices = loadVertexData("resources/vertexData/cube.txt");
    std::vector<float> chessBoardVertices = loadVertexData("resources/vertexData/chessBoard.txt");

    unsigned int woodPlaneVAO = initializeVertexArray(woodPlaneVertices);
    unsigned int lightCubeVAO = initializeVertexArray(lightCubeVertices);
    unsigned int chessBoardVAO = initializeVertexArray(chessBoardVertices);

    unsigned int woodPlaneTexture = loadTexture("resources/textures/woodFloor.png");
    
    WorldObject woodPlane(woodPlaneVAO, (int)woodPlaneVertices.size() / 8, woodPlaneTexture);
    for (int i = 0; i < 4; i++)
        pointLights[i] = WorldObject(lightCubeVAO, (int)lightCubeVertices.size() / 8);

    chessIndex.changeBoardState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    chessModel.initialize(chessBoardVAO, SCR_WIDTH, SCR_HEIGHT);
    chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);

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

        lightObjShader.use();
        lightObjShader.setMat4("projection", projection);
        lightObjShader.setMat4("view", view);
        for (int i = 0; i < pointLights.size(); i++) {
            glm::mat4 lightModel(1.0f);
            lightModel = glm::translate(lightModel, pointLightPositions[i]);
            lightModel = glm::scale(lightModel, glm::vec3(0.25f, 0.25f, 0.25f));
            lightObjShader.setMat4("model", lightModel);
            pointLights[i].draw();
        }

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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    
    /*
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
    //*/

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !pressedKeys[GLFW_KEY_T]) {
        pressedKeys[GLFW_KEY_T] = true;
        chessIndex.unMakeLastMove();
        chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);
    }

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !pressedKeys[GLFW_KEY_ENTER]) {
        pressedKeys[GLFW_KEY_ENTER] = true;
        
        chessIndex.changeBoardState("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        chessModel.updateGameData(chessIndex.mailbox, chessIndex.availableMoves, chessIndex.sideToMove);
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

    //camera.ProcessMouseMovement(xoffset, yoffset);
    
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (button == GLFW_MOUSE_BUTTON_LEFT) {

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        int clickedSquare = chessModel.getClickedSquare((int)lastX, (int)lastY, projection, view);

        std::pair<int, int> moveInfo;
        if (action == GLFW_PRESS)
            moveInfo = chessModel.processMouseClick(clickedSquare);
        else 
            moveInfo = chessModel.processMouseRelease(clickedSquare);

        if (moveInfo.first != -1) {
            Move move = chessIndex.getMove(moveInfo.first, moveInfo.second);
            chessIndex.makeMove(move, true);
            if (chessIndex.promotedPawnSquare != -1)
                chessModel.isPromotingPawn = true;
            chessModel.doMove(move, chessIndex.mailbox);
            chessModel.updateAvailableMoves(chessIndex.availableMoves);
        } 
        else if (moveInfo.second != -1) { //Pawn promotion has been selected
            chessModel.updatePromotion(chessIndex.promotedPawnSquare, moveInfo.second);
            chessIndex.updatePromotion(moveInfo.second);
            chessModel.updateAvailableMoves(chessIndex.availableMoves);
        }
    }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //camera.ProcessMouseScroll(static_cast<float>(yoffset));
}