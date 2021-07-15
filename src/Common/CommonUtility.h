#pragma once

#define RESOURCE_MESH_PLANE_SPRITE "mesh_plane_sprite"
#define RESOURCE_MESH_PLANE_WATER "mesh_plane_water"
#define RESOURCE_MESH_SAND_FLOOR "mesh_plane_sand_floor"
#define RESOURCE_MESH_PLANE_FLAT "mesh_plane_flat"
#define RESOURCE_MESH_CUBE "mesh_plane_cube"

#define RESOURCE_TEXTURE_SPARKLES "tex_sparkles"
#define RESOURCE_TEXTURE_BUBBLE_RED "tex_bubble_red"
#define RESOURCE_TEXTURE_BUBBLE_GREEN "tex_bubble_green"
#define RESOURCE_TEXTURE_BUBBLE_BLUE "tex_bubble_blue"
#define RESOURCE_TEXTURE_BUBBLE_YELLOW "tex_bubble_yellow"
#define RESOURCE_TEXTURE_BUBBLE_ORANGE "tex_bubble_orange"
#define RESOURCE_TEXTURE_BUBBLE_PURPLE "tex_bubble_purple"
#define RESOURCE_TEXTURE_BUBBLE_CYAN "tex_bubble_cyan"
#define RESOURCE_TEXTURE_WATER_DISPLACEMENT "tex_water_dm"
#define RESOURCE_TEXTURE_WATER_TEXTURE "tex_water_tm"
#define RESOURCE_TEXTURE_WORLD_1_WEM "tex_world_1_wem"
#define RESOURCE_TEXTURE_CUBEMAP_SKYBOX_1 "tex_cubemap_skybox_1"

#define RESOURCE_SHADER_COLORED_PHONG "shader_colored_phong"
#define RESOURCE_SHADER_COLORED_PHONG_2 "shader_colored_phong_2"
#define RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE "shader_textured_phong"
#define RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE_2 "shader_textured_phong_2"
#define RESOURCE_SHADER_SPRITE "shader_sprite"
#define RESOURCE_SHADER_WATER "shader_water"
#define RESOURCE_SHADER_CUBEMAP "shader_cubemap"
#define RESOURCE_SHADER_FLAT3D "shader_flat3d"

#define RESOURCE_PATH_PREFIX "path_"
#define RESOURCE_PATH_NEW_SPHERE "new_sphere"

#include <memory>
#include <Magnum/Magnum.h>
#include <Magnum/Resource.h>
#include <Magnum/ResourceManager.h>
#include <Magnum/Audio/Buffer.h>
#include <Magnum/Audio/Context.h>
#include <Magnum/Audio/Listener.h>
#include <Magnum/Audio/Source.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/CubeMapTexture.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Trade/AbstractMaterialData.h>
#include <Magnum/Shaders/Flat.h>
#include <nlohmann/json.hpp>

#include "CommonTypes.h"
#include "../Graphics/ColoredDrawable.h"
#include "../Graphics/TexturedDrawable.h"
#include "../GameObject.h"
#include "../Shaders/SpriteShader.h"

using namespace Magnum;

typedef ResourceManager<GL::Mesh, GL::Texture2D, GL::CubeMapTexture, GL::AbstractShaderProgram, Trade::AbstractMaterialData, Audio::Buffer, LinePathAsset> MyResourceManager;

class CommonUtility
{
protected:
	static const std::string VECTOR_COMPONENTS[];

public:
	static std::unique_ptr<CommonUtility> singleton;

	// Resource manager holder
	MyResourceManager manager;

	// Constructor
	CommonUtility();

	// Clear method
	void clear();

	// Read vector from JSON
	template <std::size_t S, typename T>
	const Math::Vector<S, T>& getVectorFromJson(const nlohmann::json & params)
	{
		Math::Vector<S, T> vector;
		const auto& it = params.find("position");
		if (it != params.end())
		{
			Float position[S];
			for (UnsignedInt i = 0; i < S; ++i)
			{
				(*it).at(VECTOR_COMPONENTS[i]).get_to(vector[i]);
			}
		}
		return vector;
	};

	// Audio loader
	Resource<Audio::Buffer> loadAudioData(const std::string & filename);

	// Texture loader
	Resource<GL::Texture2D> loadTexture(const std::string & filename);

	// Utilities
	void createGameSphere(GameObject* gameObject, Object3D & manipulator, const Color3 & color);
	std::shared_ptr<TexturedDrawable<SpriteShader>> createSpriteDrawable(const Int goLayerIndex, Object3D & parent, Resource<GL::Texture2D> & texture, IDrawCallback* drawCallback);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> & getFlat3DShader();

};