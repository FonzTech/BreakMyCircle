#pragma once

#include "SpriteShader.h"

struct SpriteShaderDataView
{
	SpriteShader* shader;
	SpriteShader::Parameters parameters;
	Float speed;
};