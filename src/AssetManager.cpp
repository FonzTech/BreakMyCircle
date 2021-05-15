#include "AssetManager.h"

#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/DebugStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>

#include "CommonTypes.h"

std::shared_ptr<AssetManager> AssetManager::singleton = nullptr;

void AssetManager::loadMesh(const std::string& filename)
{
	// Load a scene importer plugin
	PluginManager::Manager<Trade::AbstractImporter> manager;
	Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("AnySceneImporter");

	// Load file
	if (!importer || !importer->openFile(filename))
	{
		std::exit(4);
	}

	// Load all textures. Textures that fail to load will be NullOpt
	_textures = Containers::Array<Containers::Optional<GL::Texture2D>>{ importer->textureCount() };
	for (UnsignedInt i = 0; i != importer->textureCount(); ++i)
	{
		Debug{} << "Importing texture" << i << importer->textureName(i);

		Containers::Optional<Trade::TextureData> textureData = importer->texture(i);
		if (!textureData || textureData->type() != Trade::TextureData::Type::Texture2D)
		{
			Warning{} << "Cannot load texture properties, skipping";
			continue;
		}

		Debug{} << "Importing image" << textureData->image() << importer->image2DName(textureData->image());

		Containers::Optional<Trade::ImageData2D> imageData = importer->image2D(textureData->image());
		GL::TextureFormat format;
		if (imageData && imageData->format() == PixelFormat::RGB8Unorm)
		{
			format = GL::TextureFormat::RGB8;
		}
		else if (imageData && imageData->format() == PixelFormat::RGBA8Unorm)
		{
			format = GL::TextureFormat::RGBA8;
		}
		else
		{
			Warning{} << "Cannot load texture image, skipping";
			continue;
		}

		/* Configure the texture */
		GL::Texture2D texture;
		texture
			.setMagnificationFilter(textureData->magnificationFilter())
			.setMinificationFilter(textureData->minificationFilter(), textureData->mipmapFilter())
			.setWrapping(textureData->wrapping().xy())
			.setStorage(Math::log2(imageData->size().max()) + 1, format, imageData->size())
			.setSubImage(0, {}, *imageData)
			.generateMipmap();

		_textures[i] = std::move(texture);
	}

	/*
		Load all materials. Materials that fail to load will be NullOpt. The
		data will be stored directly in objects later, so save them only
		temporarily.
	*/
	Containers::Array<Containers::Optional<Trade::PhongMaterialData>> materials{ importer->materialCount() };
	for (UnsignedInt i = 0; i != importer->materialCount(); ++i)
	{
		Debug{} << "Importing material" << i << importer->materialName(i);

		Containers::Pointer<Trade::AbstractMaterialData> materialData = importer->material(i);
		if (!materialData || !(materialData->type() != Trade::MaterialType::Phong))
		{
			Warning{} << "Cannot load material, skipping";
			continue;
		}

		materials[i] = std::move(static_cast<Trade::PhongMaterialData&>(*materialData));
	}

	// Load all meshes. Meshes that fail to load will be NullOpt.
	_meshes = Containers::Array<Containers::Optional<GL::Mesh>>{ importer->meshCount() };
	for (UnsignedInt i = 0; i != importer->meshCount(); ++i)
	{
		Debug{} << "Importing mesh" << i << importer->meshName(i);

		Containers::Optional<Trade::MeshData> meshData = importer->mesh(i);
		if (!meshData || !meshData->hasAttribute(Trade::MeshAttribute::Normal) || meshData->primitive() != MeshPrimitive::Triangles)
		{
			Warning{} << "Cannot load the mesh, skipping";
			continue;
		}

		/* Compile the mesh */
		_meshes[i] = MeshTools::compile(*meshData);
	}

	// Load the scene
	if (importer->defaultScene() != -1)
	{
		Debug{} << "Adding default scene" << importer->sceneName(importer->defaultScene());

		Containers::Optional<Trade::SceneData> sceneData = importer->scene(importer->defaultScene());
		if (!sceneData)
		{
			Error{} << "Cannot load scene, exiting";
			return;
		}

		// Recursively add all children
		for (UnsignedInt objectId : sceneData->children3D())
		{
			// addObject(*importer, materials, RoomManager::singleton->mScene, objectId);
		}
	}
}