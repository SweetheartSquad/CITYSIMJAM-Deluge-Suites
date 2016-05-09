#pragma once

#include <Floor.h>
#include <MY_ResourceManager.h>

Floor::Floor(unsigned long int _height, Shader * _shader) : 
	height(_height)
{
	cover = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("ROOM_1")->meshes.at(0), _shader);
	addChild(cover);
	cellContainer = new Transform();
	addChild(cellContainer);
}

Floor::~Floor(){

}

void Floor::updateVisibility(unsigned long int _height){
	if(height == _height){
		setVisible(true);
		cellContainer->setVisible(true);
		cover->setVisible(false);
	}else{
		cellContainer->setVisible(false);
		if(height < _height){
			setVisible(true);
			cover->setVisible(true);
		}else{
			setVisible(false);
			cover->setVisible(false);
		}
	}
}