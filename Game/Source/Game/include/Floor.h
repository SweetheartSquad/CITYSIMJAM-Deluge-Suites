#pragma once

#include <MeshEntity.h>

#define GRID_SIZE_X 4
#define GRID_SIZE_Z (GRID_SIZE_X)

class Building;
class Timeout;
class TriMesh;
class Cell : public Node{
public:
	Building * building;
};

class Floor : public Transform{
public:
	static TriMesh * wallT1, * wallT2, * wallO1, * wallO2;
	static MeshInterface * floorPlane;
	unsigned long int height;
	std::vector<MeshEntity *> walls;
	
	Transform * cellContainer;
	Transform * wallContainerOpaque;
	Transform * wallContainerTransparent;

	Timeout * boing;

	// access is [x][z]
	Cell * cells[GRID_SIZE_X][GRID_SIZE_Z];
	
	Floor(unsigned long int _height, Shader * _shader);
	~Floor();

	void updateVisibility(unsigned long int _height, unsigned long int _angle);
};