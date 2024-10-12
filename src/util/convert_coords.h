#ifndef CONVERT_COORDINATES_H
#define CONVERT_COORDINATES_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int MAX_LOOPS = 150;

glm::vec3 convert2DCoordTo3D(int screenX, int screenY, int scr_width, int scr_height, float lowerBoundDepth, float targetY, glm::mat4& projection, glm::mat4& view) {

    glm::vec4 viewPort = glm::vec4(0, 0, scr_width, scr_height);
    glm::vec3 objCoord = glm::vec3(1.0f);

    float depth = lowerBoundDepth;
    int loops = 0;
    do {
        glm::vec3 winCoord = glm::vec3(screenX, scr_height - screenY - 1, (1.0f / depth - 10.0f) / (1.0f / 100.0f - 10.0f));
        objCoord = glm::unProject(winCoord, view, projection, viewPort);
        depth += 0.05f;
        loops++;
    } while (objCoord.y > targetY + 0.05f && loops < MAX_LOOPS);

    objCoord.y = targetY;
    return objCoord;
}

#endif