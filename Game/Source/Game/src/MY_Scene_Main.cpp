#pragma once

#include <MY_Scene_Main.h>
#include <MeshFactory.h>
#include <NumberUtils.h>
#include <RenderOptions.h>

#include <sweet/UI.h>

#include <Building.h>
#include <PointLight.h>
#include <DirectionalLight.h>

#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	currentFloor(0),
	angle(0),
	currentAngle(0),
	currentType("empty"),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true))
{


	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();


	gameCam = new OrthographicCamera(-8,8, -4.5,4.5, -100,100);
	cameras.push_back(gameCam);
	childTransform->addChild(gameCam);
	activeCamera = gameCam;

	MeshEntity * foundation = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("foundation")->meshes.at(0), baseShader);
	foundation->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ROOM_1")->texture);
	childTransform->addChild(foundation)->scale(GRID_SIZE_X, 1, GRID_SIZE_Z);
	foundation->freezeTransformation();


	buildingRoot = new Transform();
	buildingRoot->translate(0, foundation->mesh->calcBoundingBox().height, 0);
	childTransform->addChild(buildingRoot, false);
	for(unsigned long int y = 0; y < 2; ++y){
		placeFloor();
	}
	
	selectorThing = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("cube")->meshes.at(0), baseShader);
	for(auto & v : selectorThing->mesh->vertices){
		v.alpha = 0.25f;
	}
	floors.at(currentFloor)->wallContainerOpaque->addChild(selectorThing);

	gameCam->yaw = 45;
	gameCam->pitch = -45;
	gameCam->roll = 0;

	// ui stuff
	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);

	for(auto b : MY_ResourceManager::buildings->assets.at("building")){
		TextLabel * btn = new TextLabel(uiLayer->world, font, textShader);
		vl->addChild(btn);
		btn->setMouseEnabled(true);
		btn->setText(b.first);
		btn->eventManager->addEventListener("click", [this, b](sweet::Event * _event){
			setType(b.first);
		});

	}
	TextLabel * btn = new TextLabel(uiLayer->world, font, textShader);
	vl->addChild(btn);
	btn->setMouseEnabled(true);
	btn->setText("Place Floor");
	btn->eventManager->addEventListener("click", [this](sweet::Event * _event){
		placeFloor();
	});
}

MY_Scene_Main::~MY_Scene_Main(){
	// we need to destruct the scene elements before the physics world to avoid memory issues
	deleteChildTransform();
	
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
}


void MY_Scene_Main::update(Step * _step){
	// Screen shader update
	// Screen shaders are typically loaded from a file instead of built using components, so to update their uniforms
	// we need to use the OpenGL API calls
	screenSurfaceShader->bindShader(); // remember that we have to bind the shader before it can be updated
	GLint test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "time");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, _step->time);
		checkForGlError(0);
	}

	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}



	// resize camera to fit width-wise and maintain aspect ratio height-wise
	glm::uvec2 sd = sweet::getWindowDimensions();
	float ar = (float)sd.y / sd.x;
	gameCam->bottom = 16*ar * -0.5f;
	gameCam->top = 16*ar*0.5f;
	
	glm::vec3 camPos = gameCam->firstParent()->getTranslationVector();
	glm::ivec3 cursorPos = getIsometricCursorPos();
	
	selectorThing->firstParent()->translate(glm::vec3(cursorPos.x - GRID_SIZE_X/2.f, 0, cursorPos.z - GRID_SIZE_Z/2.f), false);

	if(mouse->leftJustPressed()){

		// make sure the cursor is within bounds
		if(cursorPos.x >= 0 && cursorPos.x < GRID_SIZE_X &&
			cursorPos.z >= 0 && cursorPos.z < GRID_SIZE_Z){
			
			Cell * cell = getCell(cursorPos);

			std::stringstream ss;
			ss << "Clicked floor " << currentFloor << ", cell " << cursorPos.x << " " << cursorPos.z << std::endl;
			ss << "cell type is " << cell->building->definition->id << ", user type is " << currentType;
			Log::info(ss.str());

			if(cell->building->definition->id != currentType){
				if(cell->building->definition->id == "empty" || currentType == "empty"){
					removeBuilding(cursorPos);
					placeBuilding(currentType, cursorPos);
				}
			}
		}
	}

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
	int floorChange = 0;
	if(keyboard->keyJustDown(GLFW_KEY_UP) || keyboard->keyJustDown(GLFW_KEY_W) || mouse->getMouseWheelDelta() > 0.5f){
		if(currentFloor < floors.size()-1){
			floorChange += 1;
		}
	}if(keyboard->keyJustDown(GLFW_KEY_DOWN) || keyboard->keyJustDown(GLFW_KEY_S) || mouse->getMouseWheelDelta() < -0.5f){
		if(currentFloor > 0){
			floorChange -= 1;
		}
	}
	if(floorChange != 0){
		currentFloor += glm::sign(floorChange);

		// update selector to the current floor
		selectorThing->firstParent()->firstParent()->removeChild(selectorThing->firstParent());
		floors.at(currentFloor)->wallContainerOpaque->addChild(selectorThing->firstParent());
		
		// trigger animations
		if(currentFloor != 0){
			floors.at(currentFloor-1)->boing->restart();
		}
		floors.at(currentFloor)->boing->restart();
	}
}


void MY_Scene_Main::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	// keep our screen framebuffer up-to-date with the current viewport
	screenFBO->resize(_renderOptions->viewPortDimensions.width, _renderOptions->viewPortDimensions.height);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);

	// render the scene
	_renderOptions->setClearColour(1,1,0,1);
	MY_Scene_Base::render(_matrixStack, _renderOptions);

	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	screenSurface->render(screenFBO->getTextureId());

	// render the uiLayer after the screen surface in order to avoid hiding it through shader code
	uiLayer->render(_matrixStack, _renderOptions);
}

void MY_Scene_Main::enableDebug(){
	MY_Scene_Base::enableDebug();
}
void MY_Scene_Main::disableDebug(){
	MY_Scene_Base::disableDebug();
}


glm::ivec3 MY_Scene_Main::getIsometricCursorPos(){
	// calculate in-game isometric cursor position
	glm::uvec2 sd = sweet::getWindowDimensions();
	glm::vec3 camPos = gameCam->firstParent()->getTranslationVector();
	glm::vec3 start = gameCam->screenToWorld(glm::vec3(mouse->mouseX()/sd.x, mouse->mouseY()/sd.y, gameCam->nearClip), sd);
	glm::vec3 dir = gameCam->forwardVectorRotated;
	glm::vec3 norm(0,1,0);
	glm::vec3 cursorPos(0);

	float d = glm::dot(norm, dir);
	if(glm::abs(d) > FLT_EPSILON){
		float t = glm::dot(glm::vec3(0,currentFloor,0) - start, norm) / d;
		if(glm::abs(t) > FLT_EPSILON){
			cursorPos = start + dir * t;
		}
	}
	return glm::ivec3( glm::floor(cursorPos.x) + GRID_SIZE_X/2.f, currentFloor, glm::floor(cursorPos.z) + GRID_SIZE_Z/2.f );
}

void MY_Scene_Main::setType(std::string _buildingType){
	// if the type hasn't changed, return early without doing anything
	if(currentType == _buildingType){
		return;
	}

	currentType = _buildingType;

	selectorThing->mesh->vertices.clear();
	selectorThing->mesh->indices.clear();
	selectorThing->mesh->insertVertices(*MY_ResourceManager::getBuilding(currentType)->meshes.at(0));
	for(auto & v : selectorThing->mesh->vertices){
		v.alpha = 0.25f;
	}
	selectorThing->mesh->replaceTextures(MY_ResourceManager::getBuilding(currentType)->meshes.at(0)->textures.at(0));
}

void MY_Scene_Main::placeBuilding(std::string _buildingType, glm::ivec3 _position){	
	Building * b = new Building(MY_ResourceManager::getBuilding(_buildingType), baseShader);
	floors.at(_position.y)->cellContainer->addChild(b)->translate(_position.x - GRID_SIZE_X/2.f, 0, _position.z - GRID_SIZE_Z/2.f, false);
	floors.at(_position.y)->cells[_position.x][_position.z]->building = b;

	if(b->definition->support && _position.y < floors.size()-1){
		placeBuilding("empty", _position + glm::ivec3(0,1,0));
	}
}

void MY_Scene_Main::removeBuilding(glm::ivec3 _position){
	Cell * cell = getCell(_position);
	floors.at(_position.y)->cellContainer->removeChild(cell->building->firstParent());
	delete cell->building->firstParent();
	cell->building = nullptr;
}

void MY_Scene_Main::placeFloor(){
	unsigned long int y = floors.size();
	Floor * floor = new Floor(y, baseShader);
	buildingRoot->addChild(floor, false);
	floor->translate(0, y, 0);
	floors.push_back(floor);
	for(unsigned long int x = 0; x < GRID_SIZE_X; ++x){
		for(unsigned long int z = 0; z < GRID_SIZE_Z; ++z){
			if(floors.size() == 1 || floors.at(floors.size()-2)->cells[x][z]->building->definition->support){
				placeBuilding("empty", glm::ivec3(x,y,z));
			}else{
				placeBuilding("blocked", glm::ivec3(x,y,z));
			}
		}
	}
}

Cell * MY_Scene_Main::getCell(glm::ivec3 _position){
	return floors.at(_position.y)->cells[_position.x][_position.z];
}