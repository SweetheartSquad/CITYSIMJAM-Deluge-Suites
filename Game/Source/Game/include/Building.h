#pragma once

#include <MeshEntity.h>

class AssetBuilding;

class Building : public MeshEntity{
public:
	const AssetBuilding * const definition;
	Building(const AssetBuilding * const _definition, Shader * _shader);
};