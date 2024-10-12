#ifndef ITEM_MENU_H
#define ITEM_MENU_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model/world_object.h"

#include "util/shader.h"
#include "util/read_files.h"
#include "util/convert_coords.h"
#include "util/text_render.h"

#include <vector>
#include <string>

#define TEXT		0b001 
#define BUTTON		0b010
#define TEXT_BUTTON 0b011
#define ICON		0b100
#define ICON_BUTTON 0b110

#define ORANGE glm::vec3(0.75f, 0.6f, 0.4f)
#define RED glm::vec3(0.75f, 0.4f, 0.4f)
#define GREEN glm::vec3(0.35f, 0.8f, 0.35f)
#define PURPLE glm::vec3(0.7f, 0.45f, 0.8f)
#define LIGHT_GREY glm::vec3(0.7f, 0.7f, 0.7f)
#define GREY glm::vec3(0.55f, 0.55f, 0.55f)

struct Item {
	glm::vec3 position;
	int type;
	float height;
	float width;
	std::string tag;
	bool centerAligned;
	glm::vec3 color;
	float alpha;
};

class ItemMenu {

	WorldObject background, textButtonBackground, iconButtonBackGround;
	std::vector<Item> items;
	std::vector<WorldObject> icons;
	int backgroundWidthRatio, backgroundHeightRatio;
	glm::vec3 position;
	float zRotation;
	int scr_width, scr_height;

	unsigned int squareVAO, squareTriangleCount;
	int hoveredItemID = -1;

public:

	ItemMenu() = default;

	ItemMenu(int widthRatio, int heightRatio, glm::vec3 position, float zRotation, int scr_width, int scr_height) {
		this->backgroundHeightRatio = heightRatio;
		this->backgroundWidthRatio = widthRatio;
		this->position = position;
		this->zRotation = zRotation;
		this->scr_width = scr_width;
		this->scr_height = scr_height;

		std::vector<float> squareVertexData = loadVertexData("src/resources/vertexData/square.txt");
		squareVAO = initializeVertexArray(squareVertexData);
		squareTriangleCount = squareVertexData.size() / 8;
		unsigned int blankTexture = loadTexture("src/resources/textures/blank.png");
		unsigned int fullCircleTexture = loadTexture("src/resources/textures/fullCircle.png");

		background = WorldObject(squareVAO, squareTriangleCount, blankTexture);
		textButtonBackground = WorldObject(squareVAO, squareTriangleCount, blankTexture);
		iconButtonBackGround = WorldObject(squareVAO, squareTriangleCount, fullCircleTexture);
	}

	int addItem(int type, float relativeX, float relativeZ, float width, float height, const char* tag, bool centerAligned, glm::vec3 color, float alpha) {
		Item item;
		item.position = glm::vec3(0.0f);
		item.position.x = relativeX * ((float)backgroundWidthRatio / 2.0f);
		item.position.z = relativeZ * ((float)backgroundHeightRatio / 2.0f);
		item.height = height;
		item.width = width;
		item.type = type;
		item.centerAligned = centerAligned;
		item.color = color;
		item.alpha = alpha;

		if (type & ICON) {  // Icon
			unsigned int texture = loadTexture(tag);
			WorldObject icon(squareVAO, squareTriangleCount, texture);
			icons.push_back(icon);
			item.tag = (char)('0' + icons.size() - 1); // Assumes no more than 10 icon-textures
		}
		else
			item.tag = tag;

		items.push_back(item);
		return (int)items.size() - 1;
	}

	void updateItemColor(int ID, glm::vec3 color) { items[ID].color = color; }
	void updateItemText(int ID, std::string text) { items[ID].tag = text; }
	int getHoveredItemID() { return hoveredItemID; }
	glm::vec3 getItemColor(int ID) { return items[ID].color; }

	bool hoveringItem(Item& item, glm::vec3& hoveredPosition) {
		return (
			hoveredPosition.x >= position.x + item.position.x - (item.width / 2.0f) &&
			hoveredPosition.x < position.x + item.position.x + (item.width / 2.0f) &&
			hoveredPosition.z >= position.z + item.position.z - (item.height / 2.0f) &&
			hoveredPosition.z < position.z + item.position.z + (item.height / 2.0f)
			);
	}

	void draw(Shader& shader, Shader& textShader, glm::mat4& projection, glm::mat4& view, TextRenderer& textRenderer, int mouseX, int mouseY, bool freeCameraFlight) {

		glm::vec3 hoveredPosition;
		if (!freeCameraFlight) hoveredPosition = convert2DCoordTo3D(mouseX, mouseY, scr_width, scr_height, 9.5f, 6.0f, projection, view);
		else hoveredPosition = glm::vec3(-100.0f, -100.0f, -100.0f);
		
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		bool foundHoveredItem = false;
		//Renders backgrounds of items
		for (int i = 0; i < items.size(); i++) {
			Item& item = items[i];

			if (item.type == TEXT_BUTTON) {
				shader.use();
				glm::mat4 model(1.0f);
				model = glm::translate(model, position + glm::vec3(0.0, 0.01f, 0.0f));
				model = glm::rotate(model, glm::radians(zRotation), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::translate(model, item.position);
				model = glm::scale(model, glm::vec3(item.width, 1.0f, item.height));
				shader.setMat4("model", model);
				if (hoveringItem(item, hoveredPosition)) {
					shader.setVec3("color", glm::vec3(1.0f, 0.5f, 0.5f));
					hoveredItemID = i;
					foundHoveredItem = true;
				}
				else {
					shader.setVec3("color", glm::vec3(0.3f, 0.3f, 0.3f));
					if (!foundHoveredItem) hoveredItemID = -1;
				}
				shader.setMat4("model", model);
				shader.setFloat("alpha", 0.7f);

				textButtonBackground.draw();
			}
		}
		
		glm::mat4 model(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(zRotation), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(backgroundWidthRatio, 1.0f, backgroundHeightRatio));
		shader.use();
		shader.setMat4("model", model);
		shader.setVec3("color", glm::vec3(0.0f, 0.0f, 0.0f));
		shader.setFloat("alpha", 0.5f);
		background.draw();

		//Renders the text and icons on the items
		textShader.use();
		textShader.setMat4("projection", projection);
		textShader.setMat4("view", view);
		for (int i = 0; i < items.size(); i++) {
			Item& item = items[i];
			if (item.type & TEXT) {
				textShader.use();
				renderTextItem(item, textShader, textRenderer);
			}
			else if (item.type & ICON) {
				shader.use();

				glm::mat4 modelBG(1.0f), modelIC(1.0f);
				modelBG = glm::translate(modelBG, position + glm::vec3(0.0, 0.01f, 0.0f));
				modelIC = glm::translate(modelIC, position + glm::vec3(0.0, 0.02f, 0.0f));

				modelBG = glm::rotate(modelBG, glm::radians(zRotation), glm::vec3(0.0f, 0.0f, 1.0f));
				modelIC = glm::rotate(modelIC, glm::radians(zRotation), glm::vec3(0.0f, 0.0f, 1.0f));

				modelBG = glm::translate(modelBG, item.position);
				modelIC = glm::translate(modelIC, item.position);

				modelBG = glm::scale(modelBG, glm::vec3(item.width, 1.0f, item.height));
				modelIC = glm::scale(modelIC, glm::vec3(item.width * 0.7f, 1.0f, item.height * 0.7f));
				
				shader.setMat4("model", modelBG);
				if (hoveringItem(item, hoveredPosition)) {
					shader.setVec3("color", glm::vec3(0.55f, 0.55f, 0.55f));
					shader.setFloat("alpha", 0.2f);
					hoveredItemID = i;
					foundHoveredItem = true;
				}
				else {
					shader.setVec3("color", glm::vec3(0.4f, 0.4f, 0.4f));
					shader.setFloat("alpha", 0.2f);
					if (!foundHoveredItem) hoveredItemID = -1;
				}
				iconButtonBackGround.draw();

				shader.setMat4("model", modelIC);
				shader.setVec3("color", glm::vec3(0.6f, 0.6f, 0.6f));
				shader.setFloat("alpha", 1.0f);
				icons[(int)(item.tag[0] - '0')].draw();
			}
		}
	}

	void renderTextItem(Item item, Shader& textShader, TextRenderer& textRenderer) {

		float scale = 0.006f;
		std::pair<float, float> textSize = textRenderer.getSizeOfText(item.tag, scale);
		if (textSize.first > item.width * 0.95f) {
			scale *= (item.width / textSize.first);
			if (item.type & BUTTON) scale *= 0.95f;
			textSize = textRenderer.getSizeOfText(item.tag, scale);
		}

		glm::mat4 model(1.0f);
		glm::vec3 buttonOffset = item.position;
		if (item.centerAligned) buttonOffset += glm::vec3(-textSize.first / 2.0f, 0.0f, 0.0f);
		else buttonOffset += glm::vec3(-item.width / 2.0f, 0.0f, 0.0f);

		model = glm::translate(model, position + glm::vec3(0.0f, 0.02f, textSize.second / 2.0f));
		model = glm::rotate(model, glm::radians(zRotation), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(buttonOffset)); // Then the text will rotate around its center (z-axis)
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		textShader.setMat4("model", model);
		textShader.setFloat("alpha", item.alpha);
		textRenderer.renderText(textShader, item.tag, 0.0f, 0.0f, scale, item.color);
	}

	void invertTextButtonColor(int ID, glm::vec3 color1, glm::vec3 color2) {
		if (getItemColor(ID) == color1)
			updateItemColor(ID, color2);
		else
			updateItemColor(ID, color1);
	}
};

#endif
