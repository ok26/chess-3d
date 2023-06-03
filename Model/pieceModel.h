#ifndef CHESS_PIECE_MODEL_H
#define CHESS_PIECE_MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "animation.h"

#define EMPTY 0
#define BLACK 0
#define WHITE 1

class PieceModel {

public:

	glm::vec3 position;
	Animation animation;
	glm::vec3 animationTargetPos;
	int type;
	int color;
	bool initialized = false;
	bool drawn = false;
	bool followingMouse = false;
	int movingToSquare = -1;

	PieceModel() {
		position = glm::vec3(1.0f);
		animationTargetPos = glm::vec3(1.0f);
		type = -1;
		color = WHITE;
	}

	void initialize(glm::vec3 position, int type) {
		this->position = position;
		this->type = type;
		this->initialized = true;
	}

	void followPosition(glm::mat4& model, glm::vec3 targetPosition) {
		model = glm::translate(model, targetPosition - this->position);
	}

	void moveTowardsTargetedPosition(glm::mat4& model) {
		glm::vec3 currentPositionInAnimation = (animationTargetPos - this->position) * glm::vec3(animation.animationStep);
		model = glm::translate(model, currentPositionInAnimation);
	}
};

#endif