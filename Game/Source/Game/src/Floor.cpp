#pragma once

#include <Floor.h>
#include <MY_ResourceManager.h>
#include <MeshFactory.h>
#include <Easing.h>
#include <Timeout.h>

MeshInterface * Floor::floorPlane = nullptr;
TriMesh * Floor::wallO1 = nullptr;
TriMesh * Floor::wallO2 = nullptr;
TriMesh * Floor::wallT1 = nullptr;
TriMesh * Floor::wallT2 = nullptr;
Floor::Floor(unsigned long int _height, Shader * _shader) : 
	height(_height)
{
	if(wallT1 == nullptr){
		wallO1 = new TriMesh(true);
		wallO2 = new TriMesh(true);
		wallT1 = new TriMesh(true);
		wallT2 = new TriMesh(true);
		wallO1->insertVertices(*MY_ResourceManager::globalAssets->getMesh("wall")->meshes.at(0));
		wallO2->insertVertices(*wallO1);
		wallT1->insertVertices(*wallO1);
		wallT2->insertVertices(*wallO1);

		wallO1->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ROOM_1")->texture);
		wallO2->pushTexture2D(wallO1->textures.at(0));
		wallT1->pushTexture2D(wallO1->textures.at(0));
		wallT2->pushTexture2D(wallO1->textures.at(0));

		for(auto & v : wallO1->vertices){
			v.u *= GRID_SIZE_Z;
			v.x *= GRID_SIZE_X;
			v.z *= GRID_SIZE_Z;
		}for(auto & v : wallT1->vertices){
			v.u *= GRID_SIZE_Z;
			v.x *= GRID_SIZE_X;
			v.z *= GRID_SIZE_Z;
			v.alpha = 0.25f;
		}for(auto & v : wallO2->vertices){
			v.u *= GRID_SIZE_X;
			v.x *= GRID_SIZE_Z;
			v.z *= GRID_SIZE_X;
		}for(auto & v : wallT2->vertices){
			v.u *= GRID_SIZE_X;
			v.x *= GRID_SIZE_Z;
			v.z *= GRID_SIZE_X;
			v.alpha = 0.25f;
		}
	}
	if(floorPlane == nullptr){
		floorPlane = MeshFactory::getPlaneMesh(GRID_SIZE_X/2.f, GRID_SIZE_Z/2.f);
		floorPlane->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ROOM_1")->texture);
		for(auto & v : floorPlane->vertices){
			v.z = v.y;
			v.y = 0;
			v.u *= GRID_SIZE_X;
			v.v *= GRID_SIZE_Z;
		}
	}

	wallContainerOpaque = new Transform();
	wallContainerTransparent = new Transform();
	cellContainer = new Transform();
	addChild(cellContainer,false);
	addChild(wallContainerTransparent,false);
	addChild(wallContainerOpaque,false);

	for(unsigned long int i = 0; i < 4; ++i){
		MeshEntity * wall = new MeshEntity(i%2 == 0 ? wallO1 : wallO2, _shader);
		wallContainerOpaque->addChild(wall)->rotate(i*90.f,0,1,0,kOBJECT);
		walls.push_back(wall);
		wall = new MeshEntity(i%2 == 0 ? wallT1 : wallT2, _shader);
		wallContainerTransparent->addChild(wall)->rotate(i*90.f,0,1,0,kOBJECT);
	}

	MeshEntity * fp = new MeshEntity(floorPlane, _shader);
	wallContainerOpaque->addChild(fp, false);
	
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
	boing->start();
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