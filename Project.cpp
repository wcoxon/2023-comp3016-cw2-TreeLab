
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"
#include "LoadShaders.h"

#include <glm/glm.hpp>
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#include <irrKlang.h>

#include <iostream>
#include<vector>
#include <fstream>

#include "Project.h"
#include "Mesh.h"
#include "Tree.h"

using namespace irrklang;
#pragma comment(lib, "irrKlang.lib")

#define far_plane 128.f
#define VAO_num 4

GLuint  VAOs[VAO_num];

GLuint texture1;

glm::mat4 view =  glm::mat4(1.0f);

glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3, 1.0f, far_plane);

GLuint  NumVertices = 0;

glm::vec3 lightDirection = glm::quat(glm::vec3(-0.5, 0, 0)) * glm::vec3(0, 0, -1);

GLuint trianglesProgram;
GLuint linesProgram;
GLuint instancedProgram;

std::default_random_engine generator;

Tree tree = Tree(view, projection, lightDirection);
Mesh groundMesh = Mesh(view, projection, lightDirection);

auto soundEngine = createIrrKlangDevice();

float backgroundColour[] = { 0.5f, 0.5f, 0.75f, 1.0f };
float fpsCap = 60.0f;

float sunAngle = 0.f;

bool generateMesh = false;
int segments = 4;

bool generateLeaves = false;
float leavesBranchRadius = 0.f;

float cameraZoom = 30.0f;
glm::vec3 cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraPosition = glm::vec3(0.0f, -10.0f, 0.0f);

float dragDegreesPerPixel = 360.f / 1200.f;
float dragUnitsPerPixel = 20.f / 1200.f;

std::string saveFileName{ "media/saveFile.obj" };

void init(void) {

	ShaderInfo  meshShaders[] = {
		{ GL_VERTEX_SHADER, "media/triangles.vert" },
		{ GL_FRAGMENT_SHADER, "media/triangles.frag" },
		{ GL_NONE, NULL }
	};
	trianglesProgram = LoadShaders(meshShaders);
	groundMesh.programHandle = trianglesProgram;
	tree.mesh.programHandle = trianglesProgram;

	ShaderInfo  wireframeShaders[] = {
		{ GL_VERTEX_SHADER, "media/lines.vert" },
		{ GL_FRAGMENT_SHADER, "media/triangles.frag" },
		{ GL_NONE, NULL }
	};
	linesProgram = LoadShaders(wireframeShaders);
	tree.programHandle = linesProgram;

	ShaderInfo  instancedShaders[] = {
		{ GL_VERTEX_SHADER, "media/instanced.vert" },
		{ GL_FRAGMENT_SHADER, "media/triangles.frag" },
		{ GL_NONE, NULL }
	};
	instancedProgram = LoadShaders(instancedShaders);
	tree.leaves.programHandle = instancedProgram;

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0f);

	tree.generate();

	groundMesh.loadOBJ("media/ground.obj");
	groundMesh.transform = glm::translate(glm::mat4(1.0),glm::vec3(0,3,0))*glm::scale(glm::mat4(1.0), glm::vec3(50.0));

	//creating and binding VAOs
	glGenVertexArrays(VAO_num, VAOs);

	tree.bindVAO(VAOs[0]);
	tree.loadBuffers();

	tree.mesh.bindVAO(VAOs[1]);

	groundMesh.bindVAO(VAOs[2]);
	groundMesh.loadBuffers();

	tree.leaves.bindVAO(VAOs[3]);

	//credit:
	// bark texture:
	// https://www.freepik.com/free-photo/relief-texture-brown-bark-tree-close-up_9859698.htm#query=bark%20texture&position=1&from_view=keyword&track=ais&uuid=ef27107f-8738-4d2c-a98b-195f5f2f2be7
	// leaf texture:
	// https://www.freepik.com/free-photo/nature-green-pattern-background-season_1047452.htm#query=leaf&position=2&from_view=search&track=sph&uuid=cc46b67e-7d75-45db-b832-b6efdb578607
	//soil texture:
	// https://www.freepik.com/free-photo/top-view-soil_4606525.htm#query=soil%20texture&position=1&from_view=keyword&track=ais&uuid=a018ea88-f8d5-4cb6-8a75-d7377a8b92fb

	tree.leaves.loadTexture("media/textures/nature-green-pattern-background-season.png");
	tree.mesh.loadTexture("media/textures/relief-texture-brown-bark-tree-close-up.jpg");
	groundMesh.loadTexture("media/textures/soil.jpg");
}

void updateLeaves() {
	if (generateLeaves) {
		tree.generateLeaves(leavesBranchRadius);
		tree.leaves.loadBuffers();
	}
}

void updateMesh() {

	if (generateMesh) {
		tree.generateMesh(segments);
		tree.mesh.loadBuffers();
	}
	else {
		tree.loadBuffers();
	}
}

void updateTree() {

	tree.generate();

	updateMesh();

	updateLeaves();
}

void updateView() {
	auto pull = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -cameraZoom));

	auto nod = glm::rotate(glm::mat4(1.0), glm::radians(cameraRotation.y), glm::vec3(1.0, 0.0, 0.0));
	auto swivel = glm::rotate(glm::mat4(1.0), glm::radians(cameraRotation.x), glm::vec3(0.0, 1.0, 0.0));

	auto displace = glm::translate(glm::mat4(1.0), cameraPosition);

	view = pull * nod * swivel * displace;
}

void tick(double deltaTime)
{
	ImGui::Begin("Tree Lab");
	ImGui::Text("control camera movement with WSAD\ncontrol camera angle by clicking and dragging the middle mouse button\nscroll to zoom");

	ImGui::Text("Frames Per Second: %f", 1.0/deltaTime);
	ImGui::SliderFloat("FPS target", &fpsCap, 15, 60);

	ImGui::ColorEdit3("background colour", backgroundColour);

	ImGui::SliderFloat("Light angle", &sunAngle, -3.0f, 3.0f);
	if (ImGui::IsItemEdited()) lightDirection = glm::quat(glm::vec3(0, sunAngle, 0)) * glm::quat(glm::vec3(-0.5,0,0)) * glm::vec3(0, 0, -1);

	ImGui::Text("nodes: %i", tree.nodes.size());
	ImGui::Text(
		"estimated size: %fKB",
		(
		tree.vertices.capacity() * sizeof(glm::vec3) +
		tree.normals.capacity() * sizeof(glm::vec3) +
		tree.texCoords.capacity() * sizeof(glm::vec2) +

		tree.nodes.capacity() * sizeof(Node) +
		tree.generations.capacity() * sizeof(int) +

		tree.indices.capacity() * sizeof(GLuint)
		)*1e-3
	);

	ImGui::Text("\nmodify these UI elements to change parameters used to generate the tree:");
	
	ImGui::InputInt("generation seed", &tree.seed);
	if (ImGui::IsItemEdited()) updateTree();

	ImGui::Text("thickness parameters:");
	ImGui::SliderFloat("Root radius", &tree.rootRadius, 0.0f, 3.0f);
	if (ImGui::IsItemEdited()) updateTree();
	ImGui::SliderFloat("radius decay (per unit)", &tree.rDecay, 0.01f, 0.5f);
	if (ImGui::IsItemEdited()) updateTree();

	ImGui::Text("directional parameters:");
	ImGui::SliderFloat("branch chaos", &tree.chaosLevel,0.0f,1.0f);
	if (ImGui::IsItemEdited()) updateTree();
	ImGui::SliderFloat("branch tropism", &tree.yBias, -1.0f, 1.0f);
	if (ImGui::IsItemEdited()) updateTree();

	ImGui::Text("detail settings:");
	ImGui::SliderFloat("branch length (units)", &tree.branchLength, 0.1f, 2.0f);
	if (ImGui::IsItemEdited()) updateTree();
	ImGui::SliderFloat("additional branches per unit", &tree.branchesPerUnit, 0.0f, 1.0f);
	if (ImGui::IsItemEdited()) updateTree();

	ImGui::Checkbox("generate mesh", &generateMesh);
	if (ImGui::IsItemEdited()) updateMesh();
	if (generateMesh) {


		ImGui::Text("vertices: %i", tree.mesh.vertices.size());
		ImGui::Text(
			"approx. size in memory: %fKB", 
			(
				tree.mesh.vertices.capacity()* sizeof(glm::vec3)+
				tree.mesh.normals.capacity() * sizeof(glm::vec3) +
				tree.mesh.texCoords.capacity() * sizeof(glm::vec2)+
				tree.mesh.indices.capacity()*sizeof(GLuint)
			) * 1e-3
		);

		ImGui::SliderInt("mesh resolution", &segments, 2, 16);
		if (ImGui::IsItemEdited()) updateMesh();

		ImGui::InputText("file path:", &saveFileName);

		if (ImGui::Button("Export mesh to OBJ file")) tree.mesh.saveOBJ(saveFileName);
	}

	ImGui::Checkbox("generate leaves", &generateLeaves);
	if (ImGui::IsItemEdited()) updateLeaves();
	if (generateLeaves) {
		ImGui::SliderFloat("leaf from radius", &leavesBranchRadius, 0.0f, 0.5f);
		if (ImGui::IsItemEdited()) updateLeaves();
	}


	ImGui::End();
	
	

	
	//displacement in and out from scrolling
	auto pull = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -cameraZoom));


	if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
		auto mDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);

		cameraRotation += glm::vec3(mDrag.x, mDrag.y, 0.f) * dragDegreesPerPixel;
	}
	auto nod = glm::rotate(glm::mat4(1.0), glm::radians(cameraRotation.y), glm::vec3(1.0, 0.0, 0.0));
	auto swivel = glm::rotate(glm::mat4(1.0), glm::radians(cameraRotation.x), glm::vec3(0.0, 1.0, 0.0));


	float moveSpeed = 5.f;

	glm::vec3 moveVec = glm::vec3(0);
	if (ImGui::IsKeyDown(ImGuiKey_W)) {
		moveVec += glm::vec3(0, 0, moveSpeed*deltaTime);
	}
	else if (ImGui::IsKeyDown(ImGuiKey_S)) {
		moveVec += glm::vec3(0, 0, -moveSpeed*deltaTime);
	}

	if (ImGui::IsKeyDown(ImGuiKey_A)) {
		moveVec += glm::vec3(moveSpeed*deltaTime, 0, 0);
	}
	else if (ImGui::IsKeyDown(ImGuiKey_D)) {
		moveVec += glm::vec3(-moveSpeed*deltaTime, 0, 0);
	}

	cameraPosition += glm::vec3(inverse(swivel) * glm::vec4(moveVec,1.0));


	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
		auto rDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

		glm::vec4 dragVec(rDrag.x, -rDrag.y, 0.0, 1.0);
		glm::vec3 moveVec = inverse(nod * swivel) * dragVec * dragUnitsPerPixel;

		cameraPosition += moveVec;
	}

	//translation in world space
	auto displace = glm::translate(glm::mat4(1.0), cameraPosition);

	view = pull * nod * swivel * displace;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float scrollSensitivity = 3.0f;
	cameraZoom -= yoffset * scrollSensitivity;
	updateView();
}

void display(void)
{
	glClearBufferfv(GL_COLOR, 0, backgroundColour);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (generateMesh) {
		//draw tree as mesh and ground
		tree.mesh.draw(GL_TRIANGLES);

		groundMesh.draw(GL_TRIANGLES);
	}
	else {
		//draw tree as wireframe
		tree.draw(GL_LINES);
	}

	if (generateLeaves){
		//draw leaves
		tree.leaves.drawInstanced(GL_TRIANGLES);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//update viewport size when resizing the window
	glViewport(0, 0, width, height);
	projection = glm::perspective(45.0f, (float)width / (float)height, 1.0f, far_plane);
}

int main(int argc, char** argv)
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1200, 800, "TreeLab", NULL, NULL);
	glfwMakeContextCurrent(window);

	glewInit();

	glfwSetScrollCallback(window,scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Setup Platform/Renderer bindings
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 400");


	init();
	soundEngine->play2D("G:/2022-comp3016-fp-wcoxon/media/sounds/wind.mp3", true);
	double deltaTime= 0.0;

	using namespace std::chrono;
	using secDuration = duration<double, std::ratio<1>>;

	time_point<system_clock, secDuration> frameStart = system_clock::now();
	time_point<system_clock,secDuration> frameEnd = system_clock::now();

	secDuration overshoot{ 0 };

	updateView();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		frameEnd += secDuration{ 1.0 / fpsCap };

		//sleep until frame end
		std::this_thread::sleep_until(frameEnd);
		
		//get the overshoot, how much time from the end of last frame to the start of this one.
		overshoot = secDuration(system_clock::now() - frameEnd);

		//calculate time elapsed since last call
		deltaTime = secDuration(frameEnd + overshoot - frameStart).count();

		//capture the time at the start of this frame.
		frameStart = frameEnd+overshoot;


		tick(deltaTime);
		display();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
