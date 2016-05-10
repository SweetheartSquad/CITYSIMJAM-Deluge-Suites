#pragma once

#include <MY_Scene_Base.h>
#include <Floor.h>

// A sample scene showing some of the basics of integrating a Box2D physics simulation into a game scene
class MY_Scene_Main : public MY_Scene_Base{
public:
	OrthographicCamera * gameCam;


	// the floor which the player is looking at
	int currentFloor;
	int angle;
	float currentAngle;
	
	MeshEntity * selectorThing;

	std::vector<Floor *> floors;

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();

	virtual void update(Step * _step) override;
	
	// overriden to add physics debug drawing
	virtual void enableDebug() override;
	// overriden to remove physics debug drawing
	virtual void disableDebug() override;
};