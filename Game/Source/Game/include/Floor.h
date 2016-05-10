#pragma once

#include <MeshEntity.h>

class Cell;

class Floor : public Transform{
public:
	unsigned long int height;
	std::vector<MeshEntity *> walls;
	
	Transform * cellContainer;
	Transform * wallContainer;
	Cell * cells[4][4];
	
	Floor(unsigned long int _height, Shader * _shader);
	~Floor();

	void updateVisibility(unsigned long int _height, unsigned long int _angle);
};