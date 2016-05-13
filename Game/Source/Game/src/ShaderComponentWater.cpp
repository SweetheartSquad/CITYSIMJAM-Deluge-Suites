#pragma once 

#include <ShaderComponentWater.h>
#include <shader/ShaderVariables.h>
#include <shader/ComponentShaderBase.h>
#include <MatrixStack.h>

#include <sweet.h>
#include <FileUtils.h>

ShaderComponentWater::ShaderComponentWater(ComponentShaderBase * _shader) :
	ShaderComponentMVP(_shader),
	timeUniformLocation(-1)
{
}

ShaderComponentWater::~ShaderComponentWater() {
}

std::string ShaderComponentWater::getVertexVariablesString() {
	return ShaderComponentMVP::getVertexVariablesString() + "uniform float time;" + ENDL
		+ sweet::FileUtils::readFile("assets/noise.glsl") + ENDL + "out float wave;\n";
}

std::string ShaderComponentWater::getFragmentVariablesString() {
	return ShaderComponentMVP::getFragmentVariablesString() + "uniform float time;" + ENDL
		+ sweet::FileUtils::readFile("assets/noise.glsl") + ENDL + "in float wave;\n";
}

std::string ShaderComponentWater::getVertexBodyString() {
	return
		ShaderComponentMVP::getVertexBodyString() +
		GL_IN_OUT_FRAG_UV + ".x -= mod(" + GL_IN_OUT_FRAG_UV + ".x, 1/32.f)" + SEMI_ENDL +
		GL_IN_OUT_FRAG_UV + ".y -= mod(" + GL_IN_OUT_FRAG_UV + ".y, 1/32.f)" + SEMI_ENDL +
		"wave = (sin(" + GL_IN_OUT_FRAG_UV +".x * 32 + time) * cos(" + GL_IN_OUT_FRAG_UV + ".y * 32 + time) + cnoise(vec3(" + GL_IN_OUT_FRAG_UV + " * 16, time*0.2))  + cnoise(vec3(" + GL_IN_OUT_FRAG_UV + " * 8, time*1.5))  + cnoise(vec3(" + GL_IN_OUT_FRAG_UV + " * 4, time)) )" + SEMI_ENDL +
		"gl_Position.y -= wave * 0.03;\n";
}

std::string ShaderComponentWater::getFragmentBodyString() {
	return
		ShaderComponentMVP::getFragmentBodyString() +
		"modFrag.a = wave*(cnoise(vec3(" + GL_IN_OUT_FRAG_UV + " * 15, time*1.01)) + cnoise(vec3(" + GL_IN_OUT_FRAG_UV + " * 31, time*1.29)))" + SEMI_ENDL +
		
		"modFrag.a -= mod(modFrag.a, 0.25f)" + SEMI_ENDL +
		"float d = distance(" + GL_IN_OUT_FRAG_UV + ", vec2(0.5) ) * 3.f" + SEMI_ENDL +
		"modFrag.a -= d - mod(d, 0.1f)" + SEMI_ENDL;
}

std::string ShaderComponentWater::getOutColorMod() {
	return ShaderComponentMVP::getOutColorMod();
}

void ShaderComponentWater::load() {
	if(!loaded){
		timeUniformLocation = glGetUniformLocation(shader->getProgramId(), "time");
	}
	ShaderComponentMVP::load();
}

void ShaderComponentWater::unload() {
	if(loaded){
		timeUniformLocation = -1;
	}
	ShaderComponentMVP::unload();
}

void ShaderComponentWater::configureUniforms(sweet::MatrixStack* _matrixStack, RenderOptions* _renderOption, NodeRenderable* _nodeRenderable) {
	ShaderComponentMVP::configureUniforms(_matrixStack, _renderOption, _nodeRenderable);
	glUniform1f(timeUniformLocation, sweet::lastTimestamp);
}
