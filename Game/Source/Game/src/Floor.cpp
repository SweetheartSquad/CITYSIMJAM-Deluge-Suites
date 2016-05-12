#pragma once

#include <Floor.h>
#include <MY_ResourceManager.h>
#include <Easing.h>
#include <Timeout.h>

Floor::Floor(unsigned long int _height, Shader * _shader) : 
	height(_height)
{
	wallContainerOpaque = new Transform();
	wallContainerTransparent = new Transform();
	cellContainer = new Transform();
	addChild(cellContainer,false);
	addChild(wallContainerTransparent,false);
	addChild(wallContainerOpaque,false);
	MeshInterface * wallMeshOpaque = MY_ResourceManager::globalAssets->getMesh("wallOpaque")->meshes.at(0);
	MeshInterface * wallMeshTransparent = MY_ResourceManager::globalAssets->getMesh("wallTransparent")->meshes.at(0);
	for(auto & v : wallMeshTransparent->vertices){
		v.alpha = 0.25f;
	}

	for(unsigned long int i = 0; i < 4; ++i){
		MeshEntity * wall = new MeshEntity(wallMeshOpaque, _shader);
		wallContainerOpaque->addChild(wall)->rotate(i*90.f,0,1,0,kOBJECT);
		walls.push_back(wall);
		wall = new MeshEntity(wallMeshTransparent, _shader);
		wallContainerTransparent->addChild(wall)->rotate(i*90.f,0,1,0,kOBJECT);
	}
	
	for(unsigned long int x = 0; x < GRID_SIZE_X; ++x){
	for(unsigned long int z = 0; z < GRID_SIZE_Z; ++z){
		cells[x][z] = new Cell();
		cells[x][z]->building = nullptr;
	}
	}


	boing = new Timeout(0.5f, [this](sweet::Event * _event){
		scale(1, false);
	});
	boing->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p =_event->getFloatData("progress");
		float r = 0.1f;
		float a = 0.25f;
		if(p < r){
			p = Easing::easeOutCirc(p, 1, a, r);
		}else{
			p = Easing::easeOutBounce(p - r, 1 + a, -a, 1 - r);
		}
		scale(p, 1, p, false);
	});
	addChild(boing, false);
}

Floor::~Floor(){
	for(unsigned long int x = 0; x < GRID_SIZE_X; ++x){
	for(unsigned long int z = 0; z < GRID_SIZE_Z; ++z){
		delete cells[x][z];
	}
	}
}

void Floor::updateVisibility(unsigned long int _height, unsigned long int _angle){
	
	wallContainerTransparent->setVisible(_height < height); // transparent walls visible above current floor
	wallContainerOpaque->setVisible(_height >= height); // opaque walls visible on current floor and below
	cellContainer->setVisible(_height == height); // cells visible on current floor

	if(height == _height){
		// adjust individual wall visibility on current floor based on angle
		for(unsigned long int i = 0; i < walls.size(); ++i){
			walls.at(i)->setVisible(i == _angle || i == _angle+1 || i == _angle-3);
		}
	}else{
		// make all walls visible on other floors
		for(auto wall : walls){
			wall->setVisible(true);
		}
	}
}