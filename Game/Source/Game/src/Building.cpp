#pragma once

#include <Building.h>
#include <MY_ResourceManager.h>

Building::Building(const AssetBuilding * const _definition, Shader * _shader) :
	definition(_definition),
	MeshEntity(_definition->mesh, _shader)
{
}