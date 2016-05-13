#pragma once

#include <MY_Scene_Base.h>
#include <Floor.h>

class RenderSurface;
class StandardFrameBuffer;
class TextLabel;

// A sample scene showing some of the basics of integrating a Box2D physics simulation into a game scene
class MY_Scene_Main : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	OrthographicCamera * gameCam;

	Transform * buildingRoot;
	std::string currentType;

	// the floor which the player is looking at
	int currentFloor;
	int angle;
	float currentAngle;
	
	MeshEntity * selectorThing;
	TextLabel * lblMsg;

	// handles generated resources, water level rising, etc.
	Timeout * gameplayTick;
	// reduced each tick
	// when it reaches zero, new tenants can arrive and it resets
	int tenantTimer;
	
	float food;
	float morale;
	float money;
	float weight;

	// resource generated per tick
	float moneyGen, moraleGen, foodGen;

	float waterLevel;

	float tenants;
	// maximum number of tenants
	float capacity;

	std::vector<Floor *> floors;

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();

	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;
	
	// overriden to add physics debug drawing
	virtual void enableDebug() override;
	// overriden to remove physics debug drawing
	virtual void disableDebug() override;


	// returns the isometric cursor position
	glm::ivec3 getIsometricCursorPos();

	// sets the current building type to _buildingType and updates the cursor to match
	void setType(std::string _buildingType);

	void placeBuilding(std::string _buildingType, glm::ivec3 _position, bool _free);
	void removeBuilding(glm::ivec3 _position);

	// adds a floor to the top of the building
	void placeFloor();
	void setFloor(unsigned long int _floor);

	// returns the cell at floors(_position.y)->cells[_position.x][_position.z]
	Cell * getCell(glm::ivec3 _position);


	void addTenant();
	void removeTenant();

	// alerts the player with _msg
	void alert(std::string _msg);
	Timeout * alertTimeout;
};