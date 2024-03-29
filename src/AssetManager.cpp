#include "AssetManager.h"

#include <sstream>

#include <Corrade/Utility/DebugStl.h>
#include <Corrade/Containers/PointerStl.h>
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
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Shaders/Phong.h>

#include "Common/CommonTypes.h"
#include "Common/CommonUtility.h"
#include "Graphics/GameDrawable.h"
#include "RoomManager.h"

using namespace Magnum::Math::Literals;

AssetManager::AssetManager() : AssetManager(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1)
{
}

AssetManager::AssetManager(const std::string & coloredShaderResourceKey, const std::string & texturedShaderResourceKey, const Int lightCount)
{
	// Setup colored shader
	coloredShader = getColoredShader(coloredShaderResourceKey, lightCount);

	// Setup textured shader
	texturedShader = getTexturedShader(texturedShaderResourceKey, lightCount);
}

Resource<GL::AbstractShaderProgram, Shaders::Phong> AssetManager::getColoredShader(const std::string & resourceKey, const Int lightCount)
{
	Resource<GL::AbstractShaderProgram, Shaders::Phong> resource = CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, Shaders::Phong>(resourceKey);
	if (!resource)
	{
		Shaders::Phong::Flags flags = Shaders::Phong::Flag::ObjectId;
		std::unique_ptr<Shaders::Phong> shader = std::make_unique<Shaders::Phong>(flags, lightCount);
		(*shader.get())
			.setAmbientColor(0x000000ff_rgbaf)
			.setSpecularColor(0xffffffff_rgbaf)
			.setShininess(80.0f)
			.setObjectId(0);

		Containers::Pointer<GL::AbstractShaderProgram> p = std::move((std::unique_ptr<GL::AbstractShaderProgram>&) shader);
		CommonUtility::singleton->manager.set(resource.key(), std::move(p));
	}
	return resource;
}

Resource<GL::AbstractShaderProgram, Shaders::Phong> AssetManager::getTexturedShader(const std::string & resourceKey, const Int lightCount)
{
	Resource<GL::AbstractShaderProgram, Shaders::Phong> resource = CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, Shaders::Phong>(resourceKey);
	if (!resource)
	{
		Shaders::Phong::Flags flags = Shaders::Phong::Flag::AmbientTexture | Shaders::Phong::Flag::DiffuseTexture | Shaders::Phong::Flag::AlphaMask | Shaders::Phong::Flag::ObjectId;
		std::unique_ptr<Shaders::Phong> shader = std::make_unique<Shaders::Phong>(flags, lightCount);
		(*shader.get())
			.setAmbientColor(0x000000ff_rgbaf)
			.setSpecularColor(0xffffffff_rgbaf)
			.setShininess(80.0f)
			.setObjectId(0);

		Containers::Pointer<GL::AbstractShaderProgram> p = std::move((std::unique_ptr<GL::AbstractShaderProgram>&) shader);
		CommonUtility::singleton->manager.set(resource.key(), std::move(p));
	}
	return resource;
}

void AssetManager::loadAssets(GameObject& gameObject, Object3D& manipulator, const std::string& filename, IDrawCallback* drawCallback)
{
	// Load a scene importer plugin
	PluginManager::Manager<Trade::AbstractImporter> manager;
	Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("AnySceneImporter");

	{
		const std::string fname = CommonUtility::singleton->mConfig.assetDir + filename;
		Debug{} << "Loading asset" << fname;
		if (!importer || !importer->openFile(fname))
		{
            Fatal{} << "Could not load asset" << fname;
		}
	}

	// Create asset container for this file
	ImportedAssets assets;
	assets.meshes = Containers::Array<Resource<GL::Mesh>>{ importer->meshCount() };
	assets.textures = Containers::Array<Resource<GL::Texture2D>>{ importer->textureCount() };
	assets.materials = Containers::Array<Resource<Trade::AbstractMaterialData>>{ importer->materialCount() };

	// Load all textures. Textures that fail to load will be NullOpt
	for (UnsignedInt i = 0; i != importer->textureCount(); ++i)
	{
		std::ostringstream key;
		key << "tex_" << filename << "_" << i;
		assets.textures[i] = { CommonUtility::singleton->manager.get<GL::Texture2D>(key.str()) };

		if (!assets.textures[i])
		{
			Debug{} << "Importing texture" << i << importer->textureName(i);

			Containers::Optional<Trade::TextureData> textureData = importer->texture(i);
			// if (!textureData || textureData->type() != Trade::TextureData::Type::Texture2D)
			if (!textureData)
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

			// Add to resources
			CommonUtility::singleton->manager.set(assets.textures[i].key(), std::move(texture));
		}
	}

	/*
		Load all materials. Materials that fail to load will be NullOpt. The
		data will be stored directly in objects later, so save them only
		temporarily.
	*/
	for (UnsignedInt i = 0; i != importer->materialCount(); ++i)
	{
		std::ostringstream key;
		key << "mat_" << filename << "_" << i;

		Resource<Trade::AbstractMaterialData, Trade::PhongMaterialData> amd = CommonUtility::singleton->manager.get<Trade::AbstractMaterialData, Trade::PhongMaterialData>(key.str());
		assets.materials[i] = (Resource<Trade::AbstractMaterialData>&) amd;

		if (!assets.materials[i])
		{
			Debug{} << "Importing material" << i << importer->materialName(i);

			Containers::Pointer<Trade::AbstractMaterialData> materialData = importer->material(i);
			// if (!materialData || !(materialData->type() != Trade::MaterialType::Phong))
			if (!materialData)
			{
				Warning{} << "Cannot load material, skipping";
				continue;
			}

			Trade::PhongMaterialData phongMaterial = std::move(static_cast<Trade::PhongMaterialData&>(*materialData));

			// Add to resources
			std::unique_ptr<Trade::AbstractMaterialData> material = std::make_unique<Trade::PhongMaterialData>(std::move(phongMaterial));
			Containers::Pointer<Trade::AbstractMaterialData> p = std::move(material);
			CommonUtility::singleton->manager.set(assets.materials[i].key(), std::move(p));
		}
	}

	// Load all meshes. Meshes that fail to load will be NullOpt.
	for (UnsignedInt i = 0; i != importer->meshCount(); ++i)
	{
		std::ostringstream key;
		key << "mesh_" << filename << "_" << i;
		assets.meshes[i] = { CommonUtility::singleton->manager.get<GL::Mesh>(key.str()) };

		if (!assets.meshes[i])
		{
			const auto& name = importer->meshName(i);
			Debug{} << "Importing mesh" << i << name;

			Containers::Optional<Trade::MeshData> meshData = importer->mesh(i);
			if (!meshData || !meshData->hasAttribute(Trade::MeshAttribute::Normal) || meshData->primitive() != MeshPrimitive::Triangles)
			{
				Warning{} << "Cannot load the mesh, skipping";
				continue;
			}

			// Compile the mesh
			GL::Mesh mesh = MeshTools::compile(*meshData);
			{
				const auto& p = name.find('_');
				mesh.setLabel(p != std::string::npos ? name.substr(0, p) : name);
			}

			// Add to resources
			CommonUtility::singleton->manager.set(assets.meshes[i].key(), std::move(mesh));
		}
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
		for (const UnsignedInt & objectId : sceneData->children3D())
		{
			processChildrenAssets(gameObject, assets, *importer, manipulator, objectId, drawCallback);
		}

		// Sort by trasparency
#if 1 == 2
		std::sort(gameObject.mDrawables.begin(), gameObject.mDrawables.end(), [](const std::shared_ptr<BaseDrawable>& a, const std::shared_ptr<BaseDrawable>& b) -> bool {
			if (a->mMesh.state() == ResourceState::Final && CommonUtility::singleton->stringEndsWith(a->mMesh->label(), "VT"))
			{
				a->mMesh->setLabel(a->mMesh->label() + "P");
				return true;
			}
			return false;
		});
#endif
	}
	else if (!assets.meshes.empty() && assets.meshes[0])
	{
		auto& drawables = RoomManager::singleton->mGoLayers[gameObject.mParentIndex].drawables;
		std::shared_ptr<GameDrawable<Shaders::Phong>> cd = std::make_shared<GameDrawable<Shaders::Phong>>(*drawables, coloredShader, assets.meshes[0], 0xffffffff_rgbaf);
		cd->setParent(&manipulator);
		cd->setDrawCallback(drawCallback);
		gameObject.mDrawables.emplace_back(cd);
	}
}

void AssetManager::processChildrenAssets(GameObject& gameObject, ImportedAssets& assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i, IDrawCallback* drawCallback)
{
	const std::string name = importer.object3DName(i);
	{
		Debug{} << "Importing object" << i << name;
		if (CommonUtility::singleton->stringEndsWith(name, "_AvoidMe"))
		{
			Debug{} << "This object is skipped by request";
			return;
		}
	}

	Containers::Pointer<Trade::ObjectData3D> objectData = importer.object3D(i);
	if (!objectData)
	{
		Error{} << "Cannot import object, skipping";
		return;
	}

	// Add the object to the scene and set its transformations
	auto* objectNode = new Object3D{ &parent };
	objectNode->setTransformation(objectData->transformation());

	// Add a drawable if the object has a mesh and the mesh is loaded
	if (objectData->instanceType() == Trade::ObjectInstanceType3D::Mesh && objectData->instance() != -1 && assets.meshes[objectData->instance()])
	{
		const Int materialId = static_cast<Trade::MeshObjectData3D*>(objectData.get())->material();
		auto& drawables = RoomManager::singleton->mGoLayers[gameObject.mParentIndex].drawables;

		// Material not available / not loaded, use a default material
		if (materialId == -1 || !assets.materials[materialId])
		{
			std::shared_ptr<GameDrawable<Shaders::Phong>> cd = std::make_shared<GameDrawable<Shaders::Phong>>(*drawables, coloredShader, assets.meshes[objectData->instance()], 0xffffffff_rgbaf);
			cd->setParent(objectNode);
			cd->setDrawCallback(drawCallback);
			gameObject.mDrawables.emplace_back(cd);
		}
		/*
			Textured material. If the texture failed to load, again just use a
			default colored material.
		*/
		else if (((Trade::PhongMaterialData&) *assets.materials[materialId]).flags() & Trade::PhongMaterialData::Flag::DiffuseTexture)
		{
			Resource<GL::Texture2D>& texture = assets.textures[((Trade::PhongMaterialData&) *assets.materials[materialId]).diffuseTexture()];
			if (texture)
			{
				std::shared_ptr<GameDrawable<Shaders::Phong>> td = std::make_shared<GameDrawable<Shaders::Phong>>(*drawables, texturedShader, assets.meshes[objectData->instance()], texture);
				td->setParent(objectNode);
				td->setDrawCallback(drawCallback);
				gameObject.mDrawables.emplace_back(td);
			}
			else
			{
				std::shared_ptr<GameDrawable<Shaders::Phong>> cd = std::make_shared<GameDrawable<Shaders::Phong>>(*drawables, coloredShader, assets.meshes[objectData->instance()], 0xffffffff_rgbaf);
				cd->setParent(objectNode);
				cd->setDrawCallback(drawCallback);
				gameObject.mDrawables.emplace_back(cd);
			}

		}
		// Color-only material
		else if (assets.meshes[objectData->instance()]->count() > 0)
		{
			std::shared_ptr<GameDrawable<Shaders::Phong>> cd = std::make_shared<GameDrawable<Shaders::Phong>>(*drawables, coloredShader, assets.meshes[objectData->instance()], ((Trade::PhongMaterialData&) *assets.materials[materialId]).diffuseColor());
			cd->setParent(objectNode);
			cd->setDrawCallback(drawCallback);
			gameObject.mDrawables.emplace_back(cd);
		}
#if DEBUG
		else
		{
			Debug{} << "Found mesh" << objectData->instance() << "(" << &(assets.meshes[objectData->instance()]) << ") without setCount being called on";
		}
#endif
	}

	// Recursively add children
	for (const auto& id : objectData->children())
	{
		processChildrenAssets(gameObject, assets, importer, *objectNode, UnsignedInt(id), drawCallback);
	}
}
