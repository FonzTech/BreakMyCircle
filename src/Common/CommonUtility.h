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
#include <Magnum/Math/Vector.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Trade/AbstractMaterialData.h>
#include <Magnum/Text/Text.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/DistanceFieldGlyphCache.h>
#include <nlohmann/json.hpp>

#include "CommonTypes.h"
#include "../Shaders/SpriteShader.h"
#include "../Shaders/PlasmaShader.h"
#include "../Shaders/WaterShader.h"
#include "../GameObject.h"

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

	// Base path for resources
	struct EngineConfiguration
	{
		std::string assetDir;
		Float displayDensity;
	} mConfig;

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

	/*
		Get plane mesh for specific shader. For instance, typename T
		can be "Shaders::Flat3D" and first parameter can be RESOURCE_MESH_PLANE_FLAT.
	*/
	template <typename S, typename T>
	Resource<GL::Mesh> getPlaneMeshForSpecializedShader(const std::string & rk)
	{
		// Get required resource
		Resource<GL::Mesh> resMesh{ CommonUtility::singleton->manager.get<GL::Mesh>(rk) };

		if (!resMesh)
		{
			// Create flat plane
			Trade::MeshData plane = Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates);

			GL::Buffer vertices;
			vertices.setData(MeshTools::interleave(plane.positions3DAsArray(), plane.textureCoordinates2DAsArray()));

			GL::Mesh mesh;
			mesh.setPrimitive(plane.primitive())
				.setCount(plane.vertexCount())
				.addVertexBuffer(std::move(vertices), 0, S{}, T{});

			// Add to resources
			CommonUtility::singleton->manager.set(resMesh.key(), std::move(mesh));
		}

		return resMesh;
	};

	/*
		Get plane mesh for specific shader. For instance, typename T
		can be "Shaders::Flat3D" and first parameter can be RESOURCE_SHADER_FLAT3D.
		The second parameter are whatever flags must be used during shader initialization.
		Ideally, they should be always the same for the given resource key (as first argument).
		For instance, it could be "Shaders::Flat3D::Flag::Textured | Shaders::Flat3D::Flag::AlphaMask".
	*/
	template <typename T>
	Resource<GL::AbstractShaderProgram, T> getSpecializedShader(const std::string & rk, const std::function<std::unique_ptr<GL::AbstractShaderProgram>()> & createFunction)
	{
		// Get required resource
		Resource<GL::AbstractShaderProgram, T> resShader{ CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, T>(rk) };

		if (!resShader)
		{
			// Create shader
			std::unique_ptr<GL::AbstractShaderProgram> shader = createFunction();

			// Add to resources
			Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
			CommonUtility::singleton->manager.set(resShader.key(), std::move(p));
		}

		return resShader;
	};

	// Utilities
	bool stringEndsWith(const std::string& data, const std::string& suffix);
	bool isBubbleColorValid(const Color3 & color);
	void createGameSphere(GameObject* gameObject, Object3D & manipulator, const Color3 & color);
	std::shared_ptr<BaseDrawable> createSpriteDrawable(const Int goLayerIndex, Object3D & parent, Resource<GL::Texture2D> & texture, IDrawCallback* drawCallback);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> getFlat3DShader();
	Resource<GL::AbstractShaderProgram, PlasmaShader> getPlasmaShader();
	Resource<GL::AbstractShaderProgram, WaterShader> getWaterShader();
};