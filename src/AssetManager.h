#pragma once

#include <memory>
#include <string>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/PhongMaterialData.h>

#include "CommonTypes.h"
#include "GameObject.h"

using namespace Magnum;

struct ImportedAssets
{
	Containers::Array<std::shared_ptr<GL::Mesh>> meshes;
	Containers::Array<std::shared_ptr<GL::Texture2D>> textures;
	Containers::Array<std::shared_ptr<Trade::PhongMaterialData>> materials;
};

class AssetManager
{
private:
	std::shared_ptr<Shaders::Phong> coloredShader;
	std::shared_ptr<Shaders::Phong> texturedShader;

	void processChildrenAssets(GameObject& gameObject, ImportedAssets& assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i, IDrawCallback* drawCallback);

public:
	static std::unique_ptr<AssetManager> singleton;

	AssetManager();

	void loadAssets(GameObject& gameObject, const std::string& filename, IDrawCallback* drawCallback);
};