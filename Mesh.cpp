#include "Mesh.h"

#include <iostream>
#include<vector>
#include <fstream>
#include <string>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


Mesh::Mesh(glm::mat4& _view, glm::mat4& _proj, glm::vec3& _lightDirection) {
	view = &_view;
	projection = &_proj;
	lightDirection = &_lightDirection;
}

void Mesh::bindVAO(GLuint _VAO) {
	VAOHandle = _VAO;
	glGenBuffers(NumBuffers, BufferHandles);
}

void Mesh::bufferVerts() {

	//array buffer contains vertex attributes (position, texture coordinates, normals)
	glBindBuffer(GL_ARRAY_BUFFER, BufferHandles[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() + normals.size()) * sizeof(glm::vec3) + texCoords.size() * sizeof(glm::vec2), NULL, GL_DYNAMIC_DRAW);


	// vertex positions
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
	// enable attribute
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);

	// texture coordinates
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), texCoords.size() * sizeof(glm::vec2), texCoords.data());

	glVertexAttribPointer(tPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertices.size() * sizeof(glm::vec3)));
	glEnableVertexAttribArray(tPosition);


	// bind normals to buffer
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3) + texCoords.size() * sizeof(glm::vec2), normals.size() * sizeof(glm::vec3), normals.data());

	glVertexAttribPointer(nPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertices.size() * sizeof(glm::vec3) + texCoords.size() * sizeof(glm::vec2)));
	glEnableVertexAttribArray(nPosition);

}

void Mesh::bufferIndices() {
	//index buffer, points to vertex elements in faces
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferHandles[ElementBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void Mesh::bufferInstances() {
	//instance array buffer contains transforms for instanced rendering
	glBindBuffer(GL_ARRAY_BUFFER, BufferHandles[InstanceBuffer]);
	glBufferData(GL_ARRAY_BUFFER, instanceTransforms.size() * sizeof(glm::mat4), instanceTransforms.data(), GL_DYNAMIC_DRAW);

	int transform = 4;
	glEnableVertexAttribArray(transform + 0);
	glEnableVertexAttribArray(transform + 1);
	glEnableVertexAttribArray(transform + 2);
	glEnableVertexAttribArray(transform + 3);

	glVertexAttribPointer(transform + 0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, BUFFER_OFFSET(0));
	glVertexAttribPointer(transform + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, BUFFER_OFFSET(sizeof(float) * 4));
	glVertexAttribPointer(transform + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, BUFFER_OFFSET(sizeof(float) * 8));
	glVertexAttribPointer(transform + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, BUFFER_OFFSET(sizeof(float) * 12));

	glVertexAttribDivisor(transform, 1);
	glVertexAttribDivisor(transform + 1, 1);
	glVertexAttribDivisor(transform + 2, 1);
	glVertexAttribDivisor(transform + 3, 1);
}

void Mesh::loadBuffers()
{
	glUseProgram(programHandle);
	glBindVertexArray(VAOHandle);

	bufferVerts();

	bufferIndices();

	bufferInstances();

	glBindVertexArray(0);
	glUseProgram(0);
}

void Mesh::updateBuffers() {
	glBindVertexArray(VAOHandle);

	glBindBuffer(GL_ARRAY_BUFFER, BufferHandles[ArrayBuffer]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferHandles[ElementBuffer]);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), &indices[0]);

	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(vPosition);

}

void Mesh::clearBuffers() {

	glUseProgram(programHandle);
	glBindVertexArray(VAOHandle);

	glInvalidateBufferData(BufferHandles[ArrayBuffer]);

	glInvalidateBufferData(BufferHandles[ElementBuffer]);

	glInvalidateBufferData(BufferHandles[InstanceBuffer]);

}

void Mesh::draw(GLenum mode) {

	glUseProgram(programHandle);

	int mLoc = glGetUniformLocation(programHandle, "model");
	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(transform));
	int vLoc = glGetUniformLocation(programHandle, "view");
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(*view));
	int pLoc = glGetUniformLocation(programHandle, "projection");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(*projection));

	int lLoc = glGetUniformLocation(programHandle, "lightDirection");
	glUniform3fv(lLoc, 1, glm::value_ptr(*lightDirection));


	glBindVertexArray(VAOHandle);

	glBindTexture(GL_TEXTURE_2D, texture);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferHandles[ElementBuffer]);

	glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

void Mesh::drawInstanced(GLenum mode) {

	glUseProgram(programHandle);

	int vLoc = glGetUniformLocation(programHandle, "view");
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(*view));
	int pLoc = glGetUniformLocation(programHandle, "projection");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(*projection));

	int lLoc = glGetUniformLocation(programHandle, "lightDirection");
	glUniform3fv(lLoc, 1, glm::value_ptr(*lightDirection));

	int tLoc = glGetUniformLocation(programHandle, "time");
	glUniform1f(tLoc, (float)glfwGetTime());



	glBindVertexArray(VAOHandle);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferHandles[ElementBuffer]);

	glDrawElementsInstanced(mode, indices.size(), GL_UNSIGNED_INT, 0, instanceTransforms.size());

	glBindVertexArray(0);
	glUseProgram(0);
}

void Mesh::generateNormals() {
	normals.clear();
	normals.resize(vertices.size());
	for (int i = 0; i < indices.size(); i += 3) {

		//normal of vert 0 is cross of 0->2 and 0->1
		normals[indices[i]] = glm::normalize(
			normals[indices[i]] +
			glm::normalize(glm::cross(
				(vertices[indices[i + 1]] - vertices[indices[i]]),
				(vertices[indices[i + 2]] - vertices[indices[i]])
			))
		);

		// normal 1 is ->2 x ->0
		normals[indices[i + 1]] = glm::normalize(
			normals[indices[i + 1]] +
			glm::normalize(glm::cross(
				(vertices[indices[i + 2]] - vertices[indices[i + 1]]),
				(vertices[indices[i]] - vertices[indices[i + 1]])
			))
		);

		// 2 is ->0 x ->1
		normals[indices[i + 2]] = glm::normalize(
			normals[indices[i + 2]] +
			glm::normalize(glm::cross(
				(vertices[indices[i]] - vertices[indices[i + 2]]),
				(vertices[indices[i + 1]] - vertices[indices[i + 2]])
			))
		);
	}
}

std::vector<std::string> split(std::string str, char delimiter) {
	std::vector<std::string> buffer;

	for (int i = 0; i < str.length() - 1; i = str.find(delimiter, i) + 1) {
		if (str.find(delimiter, i) == std::string::npos) {
			buffer.push_back(str.substr(i, str.length() - i));
			break;
		}
		else buffer.push_back(str.substr(i, str.find(delimiter, i) - i));
	}
	return buffer;
}

std::vector<glm::vec3> loadVertices(std::ifstream* fileStream) {
	std::vector<glm::vec3> buffer;
	std::string line;
	std::vector<std::string> lineSplit;

	while (std::getline(*fileStream, line)) {

		lineSplit = split(line, ' ');
		if (lineSplit[0] == "v") {
			buffer.push_back({ std::stof(lineSplit[1]),std::stof(lineSplit[2]), std::stof(lineSplit[3]) });
		}

	}
	return buffer;
}
std::vector<GLuint> loadIndices(std::ifstream* fileStream) {
	std::vector<GLuint> buffer;
	std::string line;
	std::vector<std::string> lineSplit;

	while (std::getline(*fileStream, line)) {

		lineSplit = split(line, ' ');

		if (lineSplit[0] == "f") {

			buffer.push_back(std::stoi(split(lineSplit[1], '/')[0]) - 1);
			buffer.push_back(std::stoi(split(lineSplit[2], '/')[0]) - 1);
			buffer.push_back(std::stoi(split(lineSplit[3], '/')[0]) - 1);
		}
	}
	return buffer;
}
std::vector<glm::vec2> loadTexCoords(std::ifstream* fileStream) {
	std::vector<glm::vec2> buffer;
	std::string line;
	std::vector<std::string> lineSplit;

	while (std::getline(*fileStream, line)) {

		lineSplit = split(line, ' ');

		if (lineSplit[0] == "vt") {

			buffer.push_back({ std::stof(lineSplit[1]),std::stof(lineSplit[2]) });
		}
	}
	return buffer;
}
std::vector<glm::vec3> loadNormals(std::ifstream* fileStream) {
	std::vector<glm::vec3> buffer;
	std::string line;
	std::vector<std::string> lineSplit;

	while (std::getline(*fileStream, line)) {

		lineSplit = split(line, ' ');

		if (lineSplit[0] == "vn") {

			buffer.push_back({ std::stof(lineSplit[1]),std::stof(lineSplit[2]), std::stof(lineSplit[3]) });
		}
	}
	return buffer;
}

void Mesh::loadTexture(std::string texturepath)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_NOTEQUAL, 0.0f); // Reject fragments with alpha == 0.0
	glEnable(GL_ALPHA_TEST);
	// load image, create texture and generate mipmaps
	GLint width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(texturepath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void Mesh::loadOBJ(std::string filePath) {

	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream) return;
	else std::cout << "file [" << filePath << "] successfully opened" << std::endl;

	vertices = loadVertices(&fileStream);

	fileStream.clear();
	fileStream.seekg(0);

	indices = loadIndices(&fileStream);

	fileStream.clear();
	fileStream.seekg(0);

	texCoords = loadTexCoords(&fileStream);

	fileStream.clear();
	fileStream.seekg(0);

	normals = loadNormals(&fileStream);

	fileStream.close();

}

void Mesh::saveOBJ(std::string filePath) {

	//ensure the mesh is being saved to an OBJ file
	if (filePath.substr(filePath.size() - 4, 4) != ".obj") return;

	std::ofstream file;
	file.open(filePath);

	//if file does not successfully open, exit the function
	if (!file.is_open()) return;

	for (int vIndex = 0; vIndex < vertices.size(); vIndex++) {
		file << "v " << vertices[vIndex].x << " " << vertices[vIndex].y << " " << vertices[vIndex].z << "\n";
	}

	for (int vtIndex = 0; vtIndex < texCoords.size(); vtIndex++) {
		file << "vt " << vertices[vtIndex].x << " " << vertices[vtIndex].y << " " << vertices[vtIndex].z << "\n";
	}

	for (int vnIndex = 0; vnIndex < normals.size(); vnIndex++) {
		file << "vn " << vertices[vnIndex].x << " " << vertices[vnIndex].y << " " << vertices[vnIndex].z << "\n";
	}

	for (int indIndex = 0; indIndex < indices.size(); indIndex += 3) {
		file << "f " << (indices[indIndex] + 1) << "/" << (indices[indIndex] + 1) << "/" << (indices[indIndex] + 1) << " " <<
			(indices[indIndex + 1] + 1) << "/" << (indices[indIndex + 1] + 1) << "/" << (indices[indIndex + 1] + 1) << " " <<
			(indices[indIndex + 2] + 1) << "/" << (indices[indIndex + 2] + 1) << "/" << (indices[indIndex + 2] + 1) << "\n";
	}

	file.close();

}