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
#include <StringUtils.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	currentFloor(0),
	angle(0),
	currentAngle(45),
	currentType("empty"),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	money(1000.f),
	morale(100),
	food(100),
	foodGen(0),
	moraleGen(-10),
	moneyGen(0),
	weight(0),
	capacity(10),
	tenants(0),
	waterLevel(0),
	tenantTimer(3),
	floodedFloors(0)
{
	// load stats from file
	Json::Reader reader;
	if(!reader.parse(sweet::FileUtils::readFile("assets/stats.json"), stats, false)){
		Log::error("JSON Parsing failed.");
	}



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

	gameCam->yaw = 45;
	gameCam->pitch = -45;
	gameCam->roll = 0;

	gameCam->update(&sweet::step);

	// ui stuff
	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	vl->setRenderMode(kTEXTURE);
	uiLayer->addChild(vl);

	for(auto b : MY_ResourceManager::buildings->assets.at("building")){
		TextLabel * btn = new TextLabel(uiLayer->world, font, textShader);
		//btn->setRenderMode(kTEXTURE);
		vl->addChild(btn);
		btn->setPadding(2);
		btn->setMargin(2);
		btn->setBackgroundColour(1,0,0,1);
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
		placeFloor(false);
	});
	
	{
	TextLabelControlled * lbl = new TextLabelControlled(&waterLevel, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "water level: ";
	lbl->suffix = " floors";
	lbl->decimals = 1;
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
	TextLabelControlled * lbl = new TextLabelControlled(&weight, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "weight: ";
	lbl->suffix = "";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
	TextLabelControlled * lbl = new TextLabelControlled(&money, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "cashmoney: ";
	lbl->suffix = " dollas";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
	TextLabelControlled * lbl = new TextLabelControlled(&food, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "food: ";
	lbl->suffix = " foods";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
	TextLabelControlled * lbl = new TextLabelControlled(&morale, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "morale: ";
	lbl->suffix = "";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
		TextLabelControlled * lbl = new TextLabelControlled(&moraleGen, -FLT_MAX, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "moraleGen: ";
	lbl->suffix = "/tick";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
	TextLabelControlled * lbl = new TextLabelControlled(&foodGen, -FLT_MAX, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "foodGen: ";
	lbl->suffix = "/tick";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}{
	TextLabelControlled * lbl = new TextLabelControlled(&moneyGen, -FLT_MAX, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "moneyGen: ";
	lbl->suffix = "/tick";
	vl->addChild(lbl);
	//lbl->setRenderMode(kTEXTURE);
	}
	{TextLabelControlled * lbl = new TextLabelControlled(&tenants, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "Population: ";
	lbl->suffix = " tenants";
	vl->addChild(lbl);
	}
	{TextLabelControlled * lbl = new TextLabelControlled(&capacity, 0, FLT_MAX, uiLayer->world, font, textShader);
	lbl->prefix = "Capacity: ";
	lbl->suffix = " tenants";
	vl->addChild(lbl);
	}
	
	lblMsg = new TextLabel(uiLayer->world, font, textShader);
	lblMsg->setText("message area");
	vl->addChild(lblMsg);

	gameplayTick = new Timeout(10.f, [this](sweet::Event * _event){
		tenantTimer -= 1;
		if(tenantTimer == 0){
			tenantTimer = 3;
			for(unsigned long int i = 0; i < sweet::NumberUtils::randomInt(1,3); ++i){
				if(capacity - tenants > 0.99f){
					addTenant();
				}
			}
		}

		money += moneyGen * getStat("tickMultipliers.money");
		morale += moraleGen * getStat("tickMultipliers.morale");
		food += foodGen * getStat("tickMultipliers.food");

		// if out of food, remainder is taken as a hit to morale and there's a chance to lose tenants
		if(food < 0){
			morale += food;
			food = 0;
			for(unsigned long int i = 0; i < glm::ceil(tenants*0.75f); ++i){
				if(sweet::NumberUtils::randomFloat() > morale/100.f){
					removeTenant();
				}
			}
		}

		float oldWaterLevel = glm::floor(waterLevel);
		waterLevel += weight * getStat("tickMultipliers.weight");
		while(waterLevel > oldWaterLevel + 0.99f){
			oldWaterLevel += 1;
			floodFloor();
		}

		gameplayTick->restart();
	});
	childTransform->addChild(gameplayTick, false);
	gameplayTick->start();

	alertTimeout = new Timeout(2.5f, [this](sweet::Event * _event){
		lblMsg->setText("");
	});
	childTransform->addChild(alertTimeout, false);





	// setup starting stats
	money = getStat("startingResources.money");
	morale = getStat("startingResources.morale");
	food = getStat("startingResources.food");
	weight = getStat("startingResources.weight");
	capacity = getStat("startingResources.capacity");
	for(unsigned long int i = 0; i < getStat("startingResources.tenants"); ++i){
		addTenant();
	}for(unsigned long int i = 0; i < getStat("startingResources.floors"); ++i){
		placeFloor(true);
	}

	
	// add the selector thing (this needs to be done after floors are made)
	selectorThing = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("cube")->meshes.at(0), baseShader);
	for(auto & v : selectorThing->mesh->vertices){
		v.alpha = 0.25f;
	}
	floors.at(currentFloor)->wallContainerOpaque->addChild(selectorThing);
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
	gameCam->bottom = gameCam->getWidth()*ar * -0.5f;
	gameCam->top = gameCam->getWidth()*ar*0.5f;
	
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
					placeBuilding(currentType, cursorPos, false);
				}else{
					alert("You can't place a building here.");
				}
			}else{
				alert("You can't place a building here.");
			}
		}
	}

	for(unsigned long int i = 0; i < floors.size(); ++i){
		floors.at(i)->updateVisibility(currentFloor + floodedFloors, angle);
	}
	camPos.y += ((currentFloor + floodedFloors) - gameCam->firstParent()->getTranslationVector().y)*0.1f;

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
		setFloor(currentFloor + glm::sign(floorChange));
	}
}

void MY_Scene_Main::setFloor(unsigned long int _floor){
	currentFloor = _floor;
	
	// update selector to the current floor
	selectorThing->firstParent()->firstParent()->removeChild(selectorThing->firstParent());
	floors.at(currentFloor)->wallContainerOpaque->addChild(selectorThing->firstParent());
		
	// trigger animations
	if(currentFloor != 0){
		floors.at(currentFloor-1)->boing->restart();
	}
	floors.at(currentFloor)->boing->restart();
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
		float t = glm::dot(glm::vec3(0,currentFloor + floodedFloors,0) - start, norm) / d;
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

void MY_Scene_Main::placeBuilding(std::string _buildingType, glm::ivec3 _position, bool _free){
	
	// if the player can't afford it, let them know and return early
	if(!_free && money - MY_ResourceManager::getBuilding(_buildingType)->cost < 0){
		alert("You can't afford this building.");
		return;
	}
	
	removeBuilding(_position);

	const AssetBuilding * ab = MY_ResourceManager::getBuilding(_buildingType);
	Building * b = new Building(ab, baseShader);
	floors.at(_position.y)->cellContainer->addChild(b)->translate(_position.x - GRID_SIZE_X/2.f, 0, _position.z - GRID_SIZE_Z/2.f, false);
	floors.at(_position.y)->cells[_position.x][_position.z]->building = b;
		
	weight += ab->weight;
	foodGen += ab->generates.food;
	moraleGen += ab->generates.morale;
	moneyGen += ab->generates.money;
	capacity += ab->capacity;

	if(b->definition->support && _position.y < floors.size()-1){
		placeBuilding("empty", _position + glm::ivec3(0,1,0), true);
	}

	if(!_free){
		money -= b->definition->cost;
	}
}

void MY_Scene_Main::removeBuilding(glm::ivec3 _position){
	Cell * cell = getCell(_position);
	if(cell->building != nullptr){
		floors.at(_position.y)->cellContainer->removeChild(cell->building->firstParent());
		const AssetBuilding * ab = cell->building->definition;
		
		weight -= ab->weight;
		foodGen -= ab->generates.food;
		moraleGen -= ab->generates.morale;
		moneyGen -= ab->generates.money;
		capacity -= ab->capacity;

		delete cell->building->firstParent();
		cell->building = nullptr;
	}
}

void MY_Scene_Main::placeFloor(bool _free){
	if(!_free){
		float cost = 100;

		// if the player can't afford it, let them know and return early
		if(money - cost < 0){
			alert("You can't afford a new floor.");
			return;
		}
		money -= 100;
	}

	unsigned long int y = floors.size();
	Floor * floor = new Floor(y, baseShader);
	buildingRoot->addChild(floor, false);
	floor->translate(0, y, 0);
	floors.push_back(floor);
	for(unsigned long int x = 0; x < GRID_SIZE_X; ++x){
		for(unsigned long int z = 0; z < GRID_SIZE_Z; ++z){
			if(floors.size() == 1 || floors.at(floors.size()-2)->cells[x][z]->building->definition->support){
				placeBuilding("empty", glm::ivec3(x,y,z), true);
			}else{
				placeBuilding("blocked", glm::ivec3(x,y,z), true);
			}
		}
	}
}

Cell * MY_Scene_Main::getCell(glm::ivec3 _position){
	return floors.at(_position.y)->cells[_position.x][_position.z];
}


void MY_Scene_Main::addTenant(){
	tenants += 1;
	
	moraleGen += -5;
	foodGen += -5;
	moneyGen += 5;
	
}
void MY_Scene_Main::removeTenant(){
	tenants -= 1;

	moraleGen -= -5;
	foodGen -= -5;
	moneyGen -= 5;

	if(tenants <= FLT_EPSILON){
		tenants = 0;
		// TODO: game over
	}
}

void MY_Scene_Main::alert(std::string _msg){
	lblMsg->setText(_msg);
	alertTimeout->restart();
}


void MY_Scene_Main::floodFloor(){
	floodedFloors += 1;

	// lose functionality of all buildings on floor
	// and a fraction of the weight
	Floor * floor = floors.front();
	for(unsigned long int x = 0; x < GRID_SIZE_X; ++x){
	for(unsigned long int z = 0; z < GRID_SIZE_X; ++z){
		const AssetBuilding * ab = floor->cells[x][z]->building->definition;
		
		weight -= ab->weight*0.5f;
		foodGen -= ab->generates.food;
		moraleGen -= ab->generates.morale;
		moneyGen -= ab->generates.money;
		capacity -= ab->capacity;
	}
	}

	// if tenants are now higher than capacity, take a morale hit
	morale -= glm::min(0.f, tenants - capacity);

	// based on morale, possibly lose some tenants
	for(unsigned long int i = 0; i < glm::ceil(tenants*0.75f); ++i){
		if(sweet::NumberUtils::randomFloat() > morale/100.f){
			removeTenant();
		}
	}

	floor->updateVisibility(floor->height+1,angle);
	floors.erase(floors.begin());


	// if this was your last floor, trigger gameover
	if(floors.size() == 0){
		// TODO: gameover
		return;
	}

	if(currentFloor == 0){
		setFloor(0);
	}else{
		setFloor(currentFloor-1);
	}

}

float MY_Scene_Main::getStat(std::string _statName){
	std::vector<std::string> sv = sweet::StringUtils::split(_statName, '.');
	Json::Value v = stats;
	for(unsigned long int i = 0; i < sv.size(); ++i){
		if(!v.isMember(sv.at(i))){
			Log::error("stat doesn't exist");
		}
		v = v[sv.at(i)];
	}
	return v.asFloat();
}