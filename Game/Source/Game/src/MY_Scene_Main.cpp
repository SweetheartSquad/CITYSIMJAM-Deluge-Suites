#pragma once

#include <MY_Scene_Main.h>
#include <MeshFactory.h>
#include <NumberUtils.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	currentFloor(0),
	angle(0),
	currentAngle(0)
{
	gameCam = new OrthographicCamera(-8,8, -4.5,4.5, -100,100);
	cameras.push_back(gameCam);
	childTransform->addChild(gameCam);
	activeCamera = gameCam;
	
	sweet::ShuffleVector<MeshInterface *> meshes;
	MY_ResourceManager::globalAssets->getMesh("ROOM_1")->meshes.at(0)->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ROOM_1")->texture);
	MY_ResourceManager::globalAssets->getMesh("stairs")->meshes.at(0)->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ROOM_1")->texture);
	MY_ResourceManager::globalAssets->getMesh("support")->meshes.at(0)->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ROOM_1")->texture);

	meshes.push(MY_ResourceManager::globalAssets->getMesh("ROOM_1")->meshes.at(0));
	meshes.push(MY_ResourceManager::globalAssets->getMesh("stairs")->meshes.at(0));
	meshes.push(MY_ResourceManager::globalAssets->getMesh("support")->meshes.at(0));
	for(unsigned long int y = 0; y < 10; ++y){
		Floor * floor = new Floor(y, baseShader);
		childTransform->addChild(floor, false);
		floor->translate(-2, y, -2);
		floors.push_back(floor);
	for(unsigned long int x = 0; x < 4; ++x){
	for(unsigned long int z = 0; z < 4; ++z){
		if(sweet::NumberUtils::randomBool()){
			MeshEntity * cube = new MeshEntity(meshes.pop(), baseShader);
			floor->cellContainer->addChild(cube)->translate(x, 0, z);
		}
	}
	}
	}

	gameCam->yaw = 45;
	gameCam->pitch = -45;
	gameCam->roll = 0;
}

MY_Scene_Main::~MY_Scene_Main(){
	// we need to destruct the scene elements before the physics world to avoid memory issues
	deleteChildTransform();
}


void MY_Scene_Main::update(Step * _step){
	glm::uvec2 sd = sweet::getWindowDimensions();
	float ar = (float)sd.y / sd.x;
	gameCam->bottom = 16*ar * -0.5f;
	gameCam->top = 16*ar*0.5f;


	for(unsigned long int i = 0; i < floors.size(); ++i){
		floors.at(i)->updateVisibility(currentFloor);
	}
	glm::vec3 camPos = gameCam->firstParent()->getTranslationVector();
	camPos.y += ((currentFloor-3.f) - gameCam->firstParent()->getTranslationVector().y)*0.1f;

	float d = (angle*90.f - currentAngle);

	currentAngle += d * 0.1f;
	gameCam->yaw = currentAngle-45.f;
	camPos.x = glm::sin(glm::radians(currentAngle)) * 5;
	camPos.z = glm::cos(glm::radians(currentAngle)) * 5;
	gameCam->firstParent()->translate(camPos, false);

	// Scene update
	MY_Scene_Base::update(_step);

	// player input
	
	// rotate view
	if(keyboard->keyJustDown(GLFW_KEY_LEFT) || keyboard->keyJustDown(GLFW_KEY_A)){
		angle -= 1;
		if(angle < 0){
			angle = 3;
			currentAngle += 360.f;
		}
	}if(keyboard->keyJustDown(GLFW_KEY_RIGHT) || keyboard->keyJustDown(GLFW_KEY_D)){
		angle += 1;
		if(angle > 3){
			angle = 0;
			currentAngle -= 360.f;
		}
	}
	// scroll floors
	if(keyboard->keyJustDown(GLFW_KEY_UP) || keyboard->keyJustDown(GLFW_KEY_W) || mouse->getMouseWheelDelta() > 0.5f){
		currentFloor += 1;
	}if(keyboard->keyJustDown(GLFW_KEY_DOWN) || keyboard->keyJustDown(GLFW_KEY_S) || mouse->getMouseWheelDelta() < -0.5f){
		if(currentFloor > 0){
			currentFloor -= 1;
		}
	}
}

void MY_Scene_Main::enableDebug(){
	MY_Scene_Base::enableDebug();
}
void MY_Scene_Main::disableDebug(){
	MY_Scene_Base::disableDebug();
}