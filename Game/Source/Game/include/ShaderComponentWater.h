#pragma once

#include <shader/ShaderComponentMVP.h>

class ShaderComponentWater : public ShaderComponentMVP{
public:

	GLint timeUniformLocation;

	ShaderComponentWater(ComponentShaderBase * _shader);
	~ShaderComponentWater();
	
	virtual std::string getVertexVariablesString() override;
	virtual std::string getFragmentVariablesString() override;
	virtual std::string getVertexBodyString() override;
	virtual std::string getFragmentBodyString() override;
	virtual std::string getOutColorMod() override;
	virtual void load() override;
	virtual void unload() override;
	virtual void configureUniforms(sweet::MatrixStack* _matrixStack, RenderOptions* _renderOption, NodeRenderable* _nodeRenderable) override;
};