#pragma once

#include <MeshEntity.h>

#define GRID_SIZE_X 4
#define GRID_SIZE_Z (GRID_SIZE_X)

class Building;
class Cell : public Node{
public:
	Building * building;
};

class Floor : public Transform{
public:
	unsigned long int height;
	std::vector<MeshEntity *> walls;
	
	Transform * cellContainer;
	Transform * wallContainer;

	// access is [x][z]
	Cell * cells[GRID_SIZE_X][GRID_SIZE_Z];
	
	Floor(unsigned long int _height, Shader * _shader);
	~Floor();

	void updateVisibility(unsigned long int _height, unsigned long int _angle);
};