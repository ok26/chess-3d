#ifndef WORLD_PLANE_H
#define WORLD_PLANE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Util/shader.h"

#include <vector>

class WorldObject {

	unsigned int triangleCount, objectVAO, objTexture;
    bool hasTexture;

public:
    WorldObject() = default;
    
	WorldObject(unsigned int objectVAO, int triangleCount, unsigned int texture) {
        this->objectVAO = objectVAO;
        this->triangleCount = triangleCount;
        this->objTexture = texture;
        hasTexture = true;
	}

    WorldObject(unsigned int objectVAO, int triangleCount) {
        this->objectVAO = objectVAO;
        this->triangleCount = triangleCount;
        this->objTexture = 0;
        hasTexture = false;
    }

    void draw() {
        if (hasTexture) 
            glBindTexture(GL_TEXTURE_2D, objTexture);

        glBindVertexArray(objectVAO);
        glDrawArrays(GL_TRIANGLES, 0, triangleCount);
    }
};

unsigned int initializeVertexArray(std::vector<float> objVertices) {

    unsigned int objectVBO, objectVAO;
    glGenVertexArrays(1, &objectVAO);
    glGenBuffers(1, &objectVBO);
    glBindVertexArray(objectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, objectVBO);
    glBufferData(GL_ARRAY_BUFFER, objVertices.size() * sizeof(float), &objVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    return objectVAO;
}


#endif