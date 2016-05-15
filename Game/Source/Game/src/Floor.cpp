#pragma once

#include <Floor.h>
#include <MY_ResourceManager.h>
#include <MeshFactory.h>
#include <Easing.h>
#include <Timeout.h>

MeshInterface * Floor::floorPlane = nullptr;
TriMesh * Floor::wallInteriorOpaque = nullptr;
TriMesh * Floor::wallInteriorTransparent = nullptr;
TriMesh * Floor::wallExterior = nullptr;
Floor::Floor(unsigned long int _height, Shader * _shader) : 
	height(_height)
{
	if(wallInteriorOpaque == nullptr){
		wallExterior = new TriMesh(true);
		wallInteriorTransparent = new TriMesh(true);
		wallInteriorOpaque = new TriMesh(true);

		wallExterior->insertVertices(*MY_ResourceManager::globalAssets->getMesh("wall")->meshes.at(0));
		for(auto & v : wallExterior->vertices){
			v.x *= GRID_SIZE_X;
			v.z *= GRID_SIZE_Z;
		}

		wallInteriorOpaque->insertVertices(*wallExterior);
		std::reverse(wallInteriorOpaque->vertices.begin(),wallInteriorOpaque->vertices.end());
		wallInteriorTransparent->insertVertices(*wallInteriorOpaque);

		wallExterior->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("wallExterior")->texture);
		wallInteriorOpaque->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("wallInterior")->texture);
		wallInteriorTransparent->pushTexture2D(wallInteriorOpaque->textures.at(0));

		for(auto & v : wallInteriorTransparent->vertices){
			v.alpha = 0.25f;
		}
	}
	if(floorPlane == nullptr){
		floorPlane = MeshFactory::getPlaneMesh(GRID_SIZE_X/2.f, GRID_SIZE_Z/2.f);
		floorPlane->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("empty")->texture);
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

	MeshEntity * fp = new MeshEntity(floorPlane, _shader);
	cellContainer->addChild(fp, false);
	cellContainer->addChild(new MeshEntity(wallInteriorOpaque, _shader));
	wallContainerOpaque->addChild(new MeshEntity(wallExterior, _shader));
	wallContainerTransparent->addChild(new MeshEntity(wallInteriorTransparent, _shader));
	
	for(unsigned long int x = 0; x < GRID_SIZE_X+2; ++x){
	for(unsigned long int z = 0; z < GRID_SIZE_Z+2; ++z){
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
	for(unsigned long int x = 0; x < GRID_SIZE_X+2; ++x){
	for(unsigned long int z = 0; z < GRID_SIZE_Z+2; ++z){
		delete cells[x][z];
	}
	}
}

#define RANGE 10
void Floor::updateVisibility(unsigned long int _height){
	setVisible(glm::abs((float)_height - height) < RANGE); // cell visible within range
	wallContainerTransparent->setVisible(_height < height || _height - height == RANGE-1); // transparent walls visible above current floor
	wallContainerOpaque->setVisible(_height > height && _height - height != RANGE-1); // opaque walls visible on floor below
	cellContainer->setVisible(_height == height); // cells visible on current floor
}