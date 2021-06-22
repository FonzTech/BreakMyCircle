#pragma once

#define RESOURCE_MESH_ICOSPHERE "mesh_icosphere"
#define RESOURCE_MESH_PLANE "mesh_plane"

#define RESOURCE_TEXTURE_SPARKLES "tex_sparkles"
#define RESOURCE_TEXTURE_BUBBLE_RED "tex_bubble_red"
#define RESOURCE_TEXTURE_BUBBLE_GREEN "tex_bubble_green"
#define RESOURCE_TEXTURE_BUBBLE_BLUE "tex_bubble_blue"
#define RESOURCE_TEXTURE_BUBBLE_YELLOW "tex_bubble_yellow"
#define RESOURCE_TEXTURE_BUBBLE_ORANGE "tex_bubble_orange"
#define RESOURCE_TEXTURE_BUBBLE_PURPLE "tex_bubble_purple"
#define RESOURCE_TEXTURE_BUBBLE_CYAN "tex_bubble_cyan"

#define RESOURCE_SHADER_COLORED_PHONG "shader_colored_phong"
#define RESOURCE_SHADER_COLORED_PHONG_2 "shader_colored_phong_2"
#define RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE "shader_textured_phong"
#define RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE_2 "shader_textured_phong_2"
#define RESOURCE_SHADER_SPRITE "shader_sprite"

#define RESOURCE_PATH_PREFIX "path_"
#define RESOURCE_PATH_NEW_SPHERE "new_sphere"

#include <memory>
#include <Magnum/Magnum.h>
#include <Magnum/Resource.h>
#include <Magnum/ResourceManager.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Trade/AbstractMaterialData.h>

#include "CommonTypes.h"
#include "ColoredDrawable.h"

using namespace Magnum;

typedef ResourceManager<GL::Mesh, GL::Texture2D, GL::AbstractShaderProgram, Trade::AbstractMaterialData, LinePathAsset> MyResourceManager;

class CommonUtility
{
public:
	static std::unique_ptr<CommonUtility> singleton;

	// Constructor
	CommonUtility();

	// Clear method
	void clear();

	// Texture loader
	Resource<GL::Texture2D> loadTexture(const std::string & filename);

	// Create game sphere
	std::shared_ptr<ColoredDrawable<Shaders::Phong>> createGameSphere(const Sint8 parentIndex, Object3D & parent, const Vector3 & diffuseColor, IDrawCallback* drawCallback);

	// Resource manager holder
	MyResourceManager manager;
};