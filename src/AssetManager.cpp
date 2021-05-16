#include "AssetManager.h"

#include <Corrade/Utility/DebugStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>

#include "CommonTypes.h"
#include "RoomManager.h"

std::shared_ptr<AssetManager> AssetManager::singleton = nullptr;

std::shared_ptr<ImportedAssets> AssetManager::loadAssets(const std::string& filename)
{
	// Load a scene importer plugin
	PluginManager::Manager<Trade::AbstractImporter> manager;
	Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("AnySceneImporter");

	// Load file
	if (!importer || !importer->openFile(filename))
	{
		std::exit(4);
	}

	// Create asset container for this file
	std::shared_ptr<ImportedAssets> assets = std::make_shared<ImportedAssets>();

	// Load all textures. Textures that fail to load will be NullOpt
	assets->textures = AssetTextures{ importer->textureCount() };
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

		assets->textures[i] = std::move(texture);
	}

	/*
		Load all materials. Materials that fail to load will be NullOpt. The
		data will be stored directly in objects later, so save them only
		temporarily.
	*/
	assets->materials = AssetMaterials{ importer->materialCount() };
	for (UnsignedInt i = 0; i != importer->materialCount(); ++i)
	{
		Debug{} << "Importing material" << i << importer->materialName(i);

		Containers::Pointer<Trade::AbstractMaterialData> materialData = importer->material(i);
		if (!materialData || !(materialData->type() != Trade::MaterialType::Phong))
		{
			Warning{} << "Cannot load material, skipping";
			continue;
		}

		assets->materials[i] = std::move(static_cast<Trade::PhongMaterialData&>(*materialData));
	}

	// Load all meshes. Meshes that fail to load will be NullOpt.
	assets->meshes = AssetMeshes{ importer->meshCount() };
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
		assets->meshes[i] = MeshTools::compile(*meshData);
	}

	// Load the scene
	if (importer->defaultScene() != -1)
	{
		Debug{} << "Adding default scene" << importer->sceneName(importer->defaultScene());

		Containers::Optional<Trade::SceneData> sceneData = importer->scene(importer->defaultScene());
		if (!sceneData)
		{
			Error{} << "Cannot load scene, exiting";
			return nullptr;
		}

		// Recursively add all children
		for (const UnsignedInt & objectId : sceneData->children3D())
		{
			processChildrenAssets(assets, *importer, RoomManager::singleton->mScene, objectId);
			return assets;
		}
	}
	else if (!assets->meshes.empty() && assets->meshes[0])
	{
		Error{} << "ColoredDrawable not supported for mesh 0";
		return assets;
	}
	return nullptr;
}

void AssetManager::processChildrenAssets(std::shared_ptr<ImportedAssets> assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i)
{
	Debug{} << "Importing object" << i << importer.object3DName(i);
	Containers::Pointer<Trade::ObjectData3D> objectData = importer.object3D(i);
	if (!objectData) {
		Error{} << "Cannot import object, skipping";
		return;
	}

	// Add the object to the scene and set its transformation
	auto* object = new Object3D{ &parent };
	object->setTransformation(objectData->transformation());

	// Add a drawable if the object has a mesh and the mesh is loaded
	if (objectData->instanceType() == Trade::ObjectInstanceType3D::Mesh && objectData->instance() != -1 && assets->meshes[objectData->instance()])
	{
		const Int materialId = static_cast<Trade::MeshObjectData3D*>(objectData.get())->material();

		// Material not available / not loaded, use a default material
		if (materialId == -1 || !assets->materials[materialId])
		{
			Error{} << "ColoredDrawable not supported for mesh " << objectData->instance();
			// new ColoredDrawable{ *object, _coloredShader, *_meshes[objectData->instance()], 0xffffff_rgbf, _drawables };

			/*
				Textured material. If the texture failed to load, again just use a
				default colored material.
			*/
		}
		else if (assets->materials[materialId]->flags() & Trade::PhongMaterialData::Flag::DiffuseTexture)
		{
			Containers::Optional<GL::Texture2D>& texture = assets->textures[assets->materials[materialId]->diffuseTexture()];
			if (texture)
			{
				// new TexturedDrawable{ *object, _texturedShader, *_meshes[objectData->instance()], *texture, _drawables };
			}
			else
			{
				Error{} << "ColoredDrawable not supported for mesh " << objectData->instance();
				// new ColoredDrawable{ *object, _coloredShader, *_meshes[objectData->instance()], 0xffffff_rgbf, _drawables };
			}

			// Color-only material
		}
		else
		{
			Error{} << "ColoredDrawable not supported for mesh " << objectData->instance();
			// new ColoredDrawable{ *object, _coloredShader, *_meshes[objectData->instance()], materials[materialId]->diffuseColor(), _drawables };
		}
	}

	// Recursively add children
	for (const std::size_t & id : objectData->children())
	{
		processChildrenAssets(assets, importer, *object, id);
	}
}