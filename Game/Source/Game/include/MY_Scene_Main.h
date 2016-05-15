#pragma once

#include <MY_Scene_Base.h>
#include <Floor.h>

class RenderSurface;
class StandardFrameBuffer;
class TextArea;
class NodeUI;

#define UI_PANEL_WIDTH 256
#define UI_PANEL_HEIGHT 512
#define UPDATE_SPACING 0.25f

// A sample scene showing some of the basics of integrating a Box2D physics simulation into a game scene
class MY_Scene_Main : public MY_Scene_Base{
public:
	Json::Value stats;
	float getStat(std::string _statName);

	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;

	OrthographicCamera * gameCam;
	float shakeMult;
	Timeout * camShake;

	Transform * buildingRoot;
	std::string currentType;

	// pause gameplay
	void pause();
	// resume gameplay
	void resume();

	// the floor which the player is looking at
	int currentFloor;
	int floodedFloors;
	// number of trips the boat has made
	unsigned long int trips;
	int angle;
	float currentAngle;
	glm::vec3 bgColour;
	
	MeshEntity * selectorThing;
	TextArea * lblMsg;
	NodeUI * arrowMoney, * arrowFood, * arrowMorale;

	// handles generated resources, water level rising, etc.
	Timeout * gameplayTick;
	// reduced each tick
	// when it reaches zero, new tenants can arrive and it resets
	int caravanTimer;

	// travels back and forth each game tick
	MeshEntity * boat;
	
	float food;
	float morale;
	float money;
	float weight;

	// resource generated per tick
	float moneyGen, moraleGen, foodGen;

	float foundationOffset;
	float waterLevel;
	MeshEntity * waterPlane;
	ComponentShaderBase * waterShader;

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
	void floodFloor();

	// returns the cell at floors(_position.y)->cells[_position.x][_position.z]
	Cell * getCell(glm::ivec3 _position);

	void updateStats();

	void addTenant();
	void removeTenant();

	// alerts the player with _msg
	void alert(std::wstring _msg);
	std::queue<Timeout *> updates;

	void changeStat(std::string _statChange, bool _positive);


	void gameOver();
};