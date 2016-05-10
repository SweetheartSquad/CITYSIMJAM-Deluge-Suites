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

	Transform * building = new Transform();
	childTransform->addChild(building, false);
	building->translate(-2, 0, -2);
	for(unsigned long int y = 0; y < 10; ++y){
		Floor * floor = new Floor(y, baseShader);
		building->addChild(floor, false);
		floor->translate(0, y, 0);
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


	sweet::setCursorMode(GLFW_CURSOR_NORMAL);

	selectorThing = new MeshEntity(MeshFactory::getCubeMesh(), baseShader);
	childTransform->addChild(selectorThing);
}

MY_Scene_Main::~MY_Scene_Main(){
	// we need to destruct the scene elements before the physics world to avoid memory issues
	deleteChildTransform();
}


void MY_Scene_Main::update(Step * _step){
	// resize camera to fit width-wise and maintain aspect ratio height-wise
	glm::uvec2 sd = sweet::getWindowDimensions();
	float ar = (float)sd.y / sd.x;
	gameCam->bottom = 16*ar * -0.5f;
	gameCam->top = 16*ar*0.5f;

	// calculate in-game isometric cursor position
	glm::vec3 camPos = gameCam->firstParent()->getTranslationVector();
	glm::vec3 start = gameCam->screenToWorld(glm::vec3(mouse->mouseX()/sd.x, mouse->mouseY()/sd.y, gameCam->nearClip), sd);
	glm::vec3 dir = gameCam->forwardVectorRotated;
	glm::vec3 norm(0,1,0);
	glm::vec3 cursorPos(0,currentFloor-1.f,0);

	float d = glm::dot(norm, dir);
	if(glm::abs(d) > FLT_EPSILON){
		float t = glm::dot(glm::vec3(0,currentFloor,0) - start, norm) / d;
		if(glm::abs(t) > FLT_EPSILON){
			cursorPos = start + dir * t;
		}
	}
	cursorPos = glm::floor(cursorPos) + glm::vec3(0.5);
	cursorPos.y = currentFloor + 0.5f;
	selectorThing->firstParent()->translate(cursorPos, false);

	for(unsigned long int i = 0; i < floors.size(); ++i){
		floors.at(i)->updateVisibility(currentFloor, angle);
	}
	camPos.y += ((currentFloor) - gameCam->firstParent()->getTranslationVector().y)*0.1f;

	float angleDif = ((angle+0.5f)*90.f - currentAngle);

	currentAngle += angleDif * 0.1f;
	gameCam->yaw = currentAngle;
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