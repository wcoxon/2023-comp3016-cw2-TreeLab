#pragma once
#include "Mesh.h"
#include <random>

struct Node {
	float radius;
	int branchCount;
};

class Tree : public Mesh {
public:

	//stores my nodes
	std::vector<Node> nodes;
	//stores the offsets of each generation of nodes
	std::vector<int> generations;

	std::default_random_engine randomEngine;

	Mesh mesh;
	Mesh leaves;


	// generation parameters ========================

	//rng seed
	int seed = 2;

	float rootRadius = 1.5f;

	//thickness decrease per unit of branch
	float rDecay = 0.1f;

	//direction jitter (per unit too? no, shorter branches don't jitter more with a given value, the jitter is pre-magnitude anyway)
	float chaosLevel = 0.2f;

	//branch length in units
	float branchLength = 1.0f;

	//new branches per unit of branch (in addition to 1)
	float branchesPerUnit = 0.2f;

	//offset branch direction by this in y
	float yBias = 0.1f;


	Tree(glm::mat4& _view, glm::mat4& _proj, glm::vec3& _lightDirection);

	void setSeed(int _seed);

	int getParentNode(int childIndex);

	int getGenerationLength(int generationIndex);

	glm::mat4 getNodeTranslation(int nodeIndex);

	glm::mat4 getNodeRotation(int nodeIndex);

	glm::mat4 getNodeScale(int nodeIndex);

	glm::mat4 getNodeTransform(int nodeIndex);

	void genBranches(int nodeIndex);

	void generateNextLayer(int generation);

	void generate();

	void generateMesh(int detail);

	void generateLeaves(float radiusThreshold);
};

