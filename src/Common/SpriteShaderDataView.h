#pragma once

#include "../Shaders/SpriteShader.h"

struct SpriteShaderDataView
{
	SpriteShader* shader;
	SpriteShader::Parameters parameters;
	Float speed;
};