#pragma once

#include <ResourceManager.h>
#include <scenario\Scenario.h>

// Template for new asset type:
class AssetBuilding : public Asset{
private:
	// constructor is private; use create instead if you need to instantiate directly
	AssetBuilding(Json::Value _json, Scenario * const _scenario);
public:
	std::vector<MeshInterface *> meshes;
	// this building can support a building on the space above it
	bool support;
	// number of tenants that can live here
	float capacity;
	// money gained/lost when placed by player
	float cost;
	// affects difficulty
	float weight;
	struct {
		float food;
		float money;
		float morale;
	} generates;

	// substitute for public constructor (we can't take the address of the constructor,
	// so we have a static function which simply returns a new instance of the class instead)
	static AssetBuilding * create(Json::Value _json, Scenario * const _scenario);
	~AssetBuilding();

	virtual void load() override;
	virtual void unload() override;
};


class MY_ResourceManager : public ResourceManager{
public:
	// A container for all of the assets which are loaded at initialization and are accessible from anywhere in the application, at any time
	static Scenario * globalAssets;
	static Scenario * buildings;
	
	MY_ResourceManager();
	~MY_ResourceManager();

	static const AssetBuilding * getBuilding(std::string _buildingAssetId);
};