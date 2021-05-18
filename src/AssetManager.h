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
	AssetMeshes meshes;
	AssetTextures textures;
	AssetMaterials materials;
};

class AssetManager
{
private:
	Shaders::Phong coloredShader;
	Shaders::Phong texturedShader;

	void processChildrenAssets(GameObject& gameObject, ImportedAssets& assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i);

public:
	static std::shared_ptr<AssetManager> singleton;

	AssetManager();

	void loadAssets(GameObject& gameObject, const std::string& filename);
};