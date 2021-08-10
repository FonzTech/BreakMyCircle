#pragma once

#include <memory>
#include <Corrade/PluginManager/Manager.h>
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
#include <Magnum/Text/Text.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/DistanceFieldGlyphCache.h>
#include <Magnum/Shaders/Flat.h>
#include <nlohmann/json.hpp>

#include "CommonTypes.h"
#include "../Graphics/ColoredDrawable.h"
#include "../Graphics/TexturedDrawable.h"
#include "../GameObject.h"
#include "../Shaders/SpriteShader.h"

using namespace Magnum;

struct FontHolder
{
	PluginManager::Manager<Text::AbstractFont> manager;
	Containers::Pointer<Text::AbstractFont> font;
	std::unique_ptr<Text::DistanceFieldGlyphCache> cache;
};

typedef ResourceManager<GL::Mesh, GL::Texture2D, GL::CubeMapTexture, GL::AbstractShaderProgram, Trade::AbstractMaterialData, Audio::Buffer, FontHolder, LinePathAsset> MyResourceManager;

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
	template <std::size_t S, class T>
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
	}

	// Audio loader
	Resource<Audio::Buffer> loadAudioData(const std::string & filename);

	// Texture loader
	Resource<GL::Texture2D> loadTexture(const std::string & filename);

	// Font loader
	Resource<FontHolder> loadFont(const std::string & filename);

	// Utilities
	bool stringEndsWith(const std::string& data, const std::string& suffix);
	void createGameSphere(GameObject* gameObject, Object3D & manipulator, const Color3 & color);
	Resource<GL::Mesh> getPlaneMeshForFlatShader();
	std::shared_ptr<TexturedDrawable<SpriteShader>> createSpriteDrawable(const Int goLayerIndex, Object3D & parent, Resource<GL::Texture2D> & texture, IDrawCallback* drawCallback);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> getFlat3DShader();
};