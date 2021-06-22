#include "CommonUtility.h"

#include <Corrade/Corrade.h>
#include <Corrade/Containers/PointerStl.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/Magnum.h>
#include <Magnum/ImageView.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Math.h>

#include "RoomManager.h"

using namespace Corrade;
using namespace Magnum;
using namespace Magnum::Math::Literals;

std::unique_ptr<CommonUtility> CommonUtility::singleton = nullptr;

CommonUtility::CommonUtility()
{
}

void CommonUtility::clear()
{
	manager.clear();
}

Resource<GL::Texture2D> CommonUtility::loadTexture(const std::string & filename)
{
	// Get sparkles texture
	Resource<GL::Texture2D> resTexture{ CommonUtility::singleton->manager.get<GL::Texture2D>(filename) };

	if (!resTexture)
	{
		// Load TGA importer plugin
		PluginManager::Manager<Trade::AbstractImporter> manager;
		Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("PngImporter");

		if (!importer || !importer->openFile("textures/" + filename.substr(4) + ".png"))
		{
			std::exit(2);
		}

		// Set texture data and parameters
		Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
		CORRADE_INTERNAL_ASSERT(image);

		GL::Texture2D texture;
		texture
			.setWrapping(GL::SamplerWrapping::ClampToEdge)
			.setMagnificationFilter(GL::SamplerFilter::Linear)
			.setMinificationFilter(GL::SamplerFilter::Linear)
			.setStorage(1, GL::textureFormat(image->format()), image->size())
			.setSubImage(0, {}, *image);

		// Add to resources
		CommonUtility::singleton->manager.set(resTexture.key(), std::move(texture));
	}

	// Return loaded resources
	return resTexture;
}

std::shared_ptr<ColoredDrawable<Shaders::Phong>> CommonUtility::createGameSphere(const Sint8 parentIndex, Object3D & parent, const Vector3 & ambientColor, IDrawCallback* drawCallback)
{
	// Check if mesh is already present
	Resource<GL::Mesh> resMesh = manager.get<GL::Mesh>(RESOURCE_MESH_ICOSPHERE);
	if (!resMesh)
	{
		// Create test mesh
		Trade::MeshData meshData = Primitives::icosphereSolid(2U);

		GL::Buffer vertices;
		vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.normalsAsArray()));

		std::pair<Containers::Array<char>, MeshIndexType> compressed = MeshTools::compressIndices(meshData.indicesAsArray());
		GL::Buffer indices;
		indices.setData(compressed.first);

		GL::Mesh mesh;
		mesh
			.setPrimitive(meshData.primitive())
			.setCount(meshData.indexCount())
			.addVertexBuffer(std::move(vertices), 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
			.setIndexBuffer(std::move(indices), 0, compressed.second);

		manager.set(resMesh.key(), std::move(mesh));
	}

	// Create Phong shader
	Resource<GL::AbstractShaderProgram, Shaders::Phong> resShader = manager.get<GL::AbstractShaderProgram, Shaders::Phong>(RESOURCE_SHADER_COLORED_PHONG);
	if (!resShader)
	{
		std::unique_ptr<GL::AbstractShaderProgram> shader = std::make_unique<Shaders::Phong>();

		Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
		manager.set(resShader.key(), std::move(p));
	}

	// Create colored drawable
	auto& drawables = RoomManager::singleton->mGoLayers[parentIndex].drawables;
	std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = std::make_shared<ColoredDrawable<Shaders::Phong>>(*drawables, resShader, resMesh, 0xffffff_rgbf);
	cd->setParent(&parent);
	cd->setDrawCallback(drawCallback);
	return cd;
}