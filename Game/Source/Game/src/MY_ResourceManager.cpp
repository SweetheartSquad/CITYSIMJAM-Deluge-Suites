#pragma once

#include <MY_ResourceManager.h>


// building
AssetBuilding::AssetBuilding(Json::Value _json, Scenario * const _scenario) :
	Asset(_json, _scenario),
	support(_json.get("support", false).asBool()),
	capacity(_json.get("capacity", 0).asFloat()),
	cost(_json.get("cost", 0).asFloat()),
	weight(_json.get("weight", 0).asFloat())
{
	generates.food = _json["generates"].get("food", 0).asFloat();
	generates.morale = _json["generates"].get("morale", 0).asFloat();
	generates.money = _json["generates"].get("money", 0).asFloat();

	Json::Value meshesJson = _json["meshes"];
	for(auto m : meshesJson){
		TriMesh * mesh = new TriMesh(true);
		mesh->insertVertices(*MY_ResourceManager::globalAssets->getMesh(m.asString())->meshes.at(0));
		mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture(_json["textures"][0].asString())->texture);
		meshes.push_back(mesh);
	}
}
AssetBuilding * AssetBuilding::create(Json::Value _json, Scenario * const _scenario){
	return new AssetBuilding(_json, _scenario);
}
AssetBuilding::~AssetBuilding(){
}

void AssetBuilding::load(){
	if(!loaded){
		//	texture->load();
	}
	Asset::load();
}

void AssetBuilding::unload(){
	if(loaded){
		//	texture->unload();
	}
	Asset::unload();
}




Scenario * MY_ResourceManager::globalAssets = nullptr;
Scenario * MY_ResourceManager::buildings = nullptr;

MY_ResourceManager::MY_ResourceManager(){
	// register custom asset types
	Asset::registerType("building", &AssetBuilding::create);

	// initialize assets
	globalAssets = new Scenario("assets/scenario.json");
	buildings = new Scenario("assets/buildings.json");
	resources.push_back(globalAssets);
	resources.push_back(buildings);

	load();
}

MY_ResourceManager::~MY_ResourceManager(){
	unload();
	destruct();
}

const AssetBuilding * MY_ResourceManager::getBuilding(std::string _buildingAssetId){
	return dynamic_cast<AssetBuilding *>(buildings->getAsset("building", _buildingAssetId));
}