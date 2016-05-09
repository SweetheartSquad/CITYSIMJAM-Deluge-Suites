#pragma once

#include <MeshEntity.h>

class Cell;

class Floor : public Transform{
public:
	unsigned long int height;
	MeshEntity * cover;

	Transform * cellContainer;
	Cell * cells[4][4];
	
	Floor(unsigned long int _height, Shader * _shader);
	~Floor();

	void updateVisibility(unsigned long int _height);
};