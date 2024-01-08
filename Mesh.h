#pragma once

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include<vector>
#include <fstream>

enum Buffer_IDs { ArrayBuffer, ElementBuffer, InstanceBuffer, NumBuffers = 3 };
enum Attrib_IDs { vPosition = 0, cPosition = 1, tPosition = 2, nPosition = 3 };

#define BUFFER_OFFSET(a) ((void*)(a))

class  Mesh {
public:
	GLuint programHandle;
	GLuint VAOHandle;
	GLuint BufferHandles[NumBuffers];

	GLuint texture;

	glm::mat4* view;
	glm::mat4* projection;
	glm::vec3* lightDirection;

	glm::mat4 transform = glm::mat4(1.0f);

	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indices;
	std::vector<glm::vec3> normals = { {0.0f,1.0f,0.0f}, };
	std::vector<glm::vec2> texCoords = { {0.0f,0.0f},{1.0f,0.0f},{1.0f,1.0f} };

	std::vector<glm::mat4> instanceTransforms = { glm::mat4(1.0) };

	Mesh(glm::mat4& _view, glm::mat4& _proj, glm::vec3& _lightDirection);

	void bindVAO(GLuint _VAO);

	void bufferVerts();

	void bufferIndices();

	void bufferInstances();

	void loadBuffers();

	void updateBuffers();

	void clearBuffers();

	void draw(GLenum mode);

	void drawInstanced(GLenum mode);

	void generateNormals();

	void loadTexture(std::string texturepath);

	void loadOBJ(std::string filePath);

	void saveOBJ(std::string filePath);
};

