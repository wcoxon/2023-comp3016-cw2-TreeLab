#include "Tree.h"

Tree::Tree(glm::mat4& _view, glm::mat4& _proj, glm::vec3& _lightDirection) : Mesh(_view,_proj,_lightDirection), mesh(Mesh(_view, _proj, _lightDirection)), leaves(Mesh(_view, _proj, _lightDirection)) 
{
	leaves.vertices = {
		{-0.25,0.0,0.0},
		{0.25,0.0,0.0},
		{-0.25,1.0,0.0},
		{0.25,1.0,0.0}
	};
	leaves.indices = {
		1,0,2,
		2,3,1,
		2,0,1,
		1,3,2
	};
	leaves.texCoords = {
		{0.0,0.0},
		{1.0,0.0},
		{0.0,1.0},
		{1.0,1.0}
	};
	leaves.normals = {
		{0.0,0.0,-1.0},
		{0.0,0.0,-1.0},
		{0.0,0.0,-1.0},
		{0.0,0.0,-1.0}
	};
}

void Tree::setSeed(int _seed) { seed = _seed; }

int Tree::getParentNode(int childIndex) {
	if (childIndex == 0) return -1;
	return indices[(childIndex - 1) * 2];
}

int Tree::getGenerationLength(int generationIndex) {
	//if indexing the last generation, use verts count as end
	if (generations.size() - 1 == generationIndex) return vertices.size() - generations[generationIndex];
	return generations[generationIndex + 1] - generations[generationIndex];
}

glm::mat4 Tree::getNodeTranslation(int nodeIndex) {
	return glm::translate(
		glm::mat4(1.0f),
		vertices[nodeIndex]
	);
}

glm::mat4 Tree::getNodeRotation(int nodeIndex) {
	glm::vec3 direction = glm::normalize(vertices[nodeIndex] - vertices[getParentNode(nodeIndex)]);
	return (direction.y == 1) ? glm::mat4(1.0) : glm::rotate(
		glm::mat4(1.0f),
		std::acos(direction.y),
		glm::cross(glm::vec3(0.0, 1.0, 0.0),direction)
	);
}

glm::mat4 Tree::getNodeScale(int nodeIndex) {
	return glm::scale(
		glm::mat4(1.0f),
		glm::vec3(nodes[nodeIndex].radius)
	);
}

glm::mat4 Tree::getNodeTransform(int nodeIndex) {
	return getNodeTranslation(nodeIndex) * getNodeScale(nodeIndex) * getNodeRotation(nodeIndex);
}

void Tree::genBranches(int nodeIndex) {

	//setting number of branches for child node
	if (nodes[nodeIndex].radius > 0) {

		float currentDepth = generations.size() * branchLength;
		float previousDepth = currentDepth - branchLength;

		//if the number of branches up to this node is greater than the number of branches up to the previous, then we have a branch on this node
		float newBranches = int(currentDepth * branchesPerUnit) - int(previousDepth * branchesPerUnit);
		nodes[nodeIndex].branchCount = 1 + newBranches;
	}
	else {
		nodes[nodeIndex].radius = 0;
		nodes[nodeIndex].branchCount = 0;
		return;
	}

	//calculate direction of parent branch
	glm::vec3 direction = (nodeIndex == 0) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::normalize(vertices[nodeIndex] - vertices[getParentNode(nodeIndex)]);

	glm::quat directionRotation = (direction.y == 1) ? glm::identity<glm::quat>() : glm::angleAxis(std::acos(direction.y), glm::cross(glm::vec3(0, 1, 0), direction));//(1.f-direction.y <0.001 ) ? glm::identity<glm::quat>() : glm::quatLookAt(direction, glm::vec3(0.f, 1.f, 0.f));


	// a 1->0 normalised curve on nodes radius
	float radiusRamp = 0.8 * std::pow(std::sin(glm::half_pi<float>() * nodes[nodeIndex].radius / rootRadius), 2) + 0.2;

	float childChaos = (1 - radiusRamp) * chaosLevel;

	float childDecay = rDecay * radiusRamp;

	float childRadius = std::max(nodes[nodeIndex].radius - childDecay * branchLength, 0.0f);

	float length = (nodes[nodeIndex].radius - childRadius) / childDecay;

	for (float branchIndex = 0; branchIndex < nodes[nodeIndex].branchCount; branchIndex++) {

		double randomAngle = std::uniform_real_distribution<double>(0.0, glm::two_pi<double>())(randomEngine);
		float chaosAngle = childChaos * glm::pi<float>();

		//make a rotation about an xy vector
		glm::quat noise = glm::angleAxis(chaosAngle, directionRotation * glm::vec3(cos(randomAngle), 0, -sin(randomAngle)));

		glm::quat tropism = (direction.y == 1) ? glm::identity<glm::quat>() : glm::angleAxis(-yBias, glm::cross(glm::vec3(0, 1, 0), direction));

		glm::vec3 displacement = tropism * noise * direction * length;

		//add new node
		nodes.push_back(Node{ childRadius });

		//adding new node's vertex
		vertices.push_back(vertices[nodeIndex] + displacement);

		//adding edge/branch indices into new node
		indices.push_back(nodeIndex);
		indices.push_back(vertices.size() - 1);
	}
}

void Tree::generateNextLayer(int generation) {

	//if there are no nodes in this generation, return (we can't make any nodes from this)
	if (generations[generation] >= vertices.size()) return;

	//add the end of the vertices as the start for the next generation
	generations.push_back(vertices.size());

	//from the start to the end of this generation, branch from each node
	for (int nodeIndex = generations[generation]; nodeIndex < generations[generation + 1]; nodeIndex++) {
		genBranches(nodeIndex);
	}
}

void Tree::generate() {

	randomEngine.seed(seed);

	//first generation offset is 0
	generations = { 0 };

	nodes = { Node{rootRadius,1} };

	vertices = { {0.0f,0.0f,0.0f} };
	indices = {};

	float genStart = glfwGetTime();

	for (int generation = 0; generation < generations.size(); generation++) {
		generateNextLayer(generation);
	}

	float genDuration = glfwGetTime() - genStart;
}

void Tree::generateMesh(int detail) {

	//texture height in units
	float textureHeight = 1.0;
	float textureWidth = 5.0;

	float genStart = glfwGetTime();


	//calculating number of vertices in this mesh to allocate memory for
	int vertCount = generations[generations.size() - 2] * (detail + 1) + (nodes.size() - generations[generations.size() - 2]);

	mesh.vertices.clear();
	mesh.vertices.reserve(vertCount);

	mesh.normals.clear();
	mesh.normals.reserve(vertCount);

	mesh.texCoords.clear();
	mesh.normals.reserve(vertCount);

	mesh.indices.clear();
	mesh.indices.reserve((detail+1) * generations[generations.size()-2]*6 + (nodes.size() - generations[generations.size()-2]) * (detail + 1)*3);


	//create circle for root node
	for (int vert = 0; vert < (detail + 1); vert++) {
		float sector = (float)vert / detail;
		float angle = glm::two_pi<float>() * sector;

		mesh.vertices.push_back(glm::vec3(getNodeScale(0) * glm::vec4(cos(angle), 0.0f, sin(angle), 1.0f)));
		mesh.normals.push_back(glm::vec3(cos(angle), 0.0f, sin(angle)));

		mesh.texCoords.push_back({ angle * nodes[0].radius / textureWidth, 0 });
	}

	//actual index of current vertex in mesh (not the last vertex, but the one to make faces from basically)
	unsigned int meshOffset = 0;

	//no need to process nodes with no branches
	int terminalNodes = generations[generations.size() - 1];

	int childNodeIndex = 0;

	for (int nodeIndex = 0; nodeIndex < terminalNodes; nodeIndex++) {

		for (int branchIndex = 0; branchIndex < nodes[nodeIndex].branchCount; branchIndex++) {
			childNodeIndex++;
			glm::mat4 childTransform = getNodeTransform(childNodeIndex);


			glm::mat3 childRotate = getNodeRotation(childNodeIndex);

			unsigned int childCircleOffset = mesh.vertices.size();

			//if dead end branch, add 1 vertex and make triangles to the point
			if (nodes[childNodeIndex].branchCount == 0) {
				mesh.vertices.push_back(vertices[childNodeIndex]);
				mesh.normals.push_back(childRotate * glm::vec3(0, 1, 0));
				mesh.texCoords.push_back({ 0.5f,1.0f });
				for (int v = 0; v < detail + 1; v++) {

					mesh.indices.insert(mesh.indices.end(), {
						meshOffset + (v + 1) % (detail + 1),
						meshOffset + v,
						childCircleOffset,
						});

				}
				continue;
			}


			//else create a circle on child node and add faces connecting between circles
			for (int circleVertex = 0; circleVertex < (detail + 1); circleVertex++) {

				//interpolate between 0 and 1 around the circle
				float sector = (float)circleVertex / detail;

				//interpolate between 0 and 2pi
				//clockwise btw
				float angle = glm::two_pi<float>() * sector;

				//generate vertex positions and normals at each angle

				auto vertexDirection = glm::vec3(cos(angle), 0.0f, sin(angle));

				mesh.vertices.push_back(glm::vec3(childTransform * glm::vec4(vertexDirection, 1.0f)));
				mesh.normals.push_back(childRotate * vertexDirection);


				//    # this has issues now due to non-constant decay
				float depthUnits = (nodes[0].radius - nodes[childNodeIndex].radius) / rDecay;

				mesh.texCoords.push_back({
					angle * nodes[childNodeIndex].radius / textureWidth,
					depthUnits / textureHeight
					});


				//make a quad face between this segment of the circles (CCW is set as front face)
				mesh.indices.insert(mesh.indices.end(), {
					//bottom left ->
					meshOffset + (circleVertex + 1) % (detail + 1), // detail plus one because we have an extra vertex at the start position with a different texture coordinate
					//bottom right ->
					meshOffset + circleVertex,
					//top right ->
					childCircleOffset + circleVertex,

					//top right ->
					childCircleOffset + circleVertex,
					//top left ->
					childCircleOffset + (circleVertex + 1) % (detail + 1),
					//bottom left
					meshOffset + (circleVertex + 1) % (detail + 1)
					}
				);
			}
		}
		meshOffset += detail + 1;
	}
}

void Tree::generateLeaves(float radiusThreshold) {

	leaves.instanceTransforms.clear();

	for (int nodeIndex = 1; nodeIndex < nodes.size(); nodeIndex++) {
		if (nodes[nodeIndex].radius > radiusThreshold) continue;

		glm::vec3 direction = glm::normalize(vertices[nodeIndex] - vertices[getParentNode(nodeIndex)]);

		leaves.instanceTransforms.push_back(
			getNodeTranslation(nodeIndex) * 
			glm::rotate(glm::mat4(1.0), std::atan2(direction.x, direction.z), glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0), std::acos(direction.y)/2.0f+ glm::half_pi<float>(), glm::vec3(1, 0,0))
		);

	}
}