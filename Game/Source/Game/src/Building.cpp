#pragma once

#include <Building.h>
#include <MY_ResourceManager.h>

#include <NumberUtils.h>

Building::Building(const AssetBuilding * const _definition, Shader * _shader) :
	definition(_definition),
	MeshEntity(sweet::NumberUtils::randomItem(_definition->meshes), _shader)
{
}