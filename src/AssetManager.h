#pragma once

#include <memory>
#include <string>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/PhongMaterialData.h>

#include "CommonTypes.h"

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
	void processChildrenAssets(std::shared_ptr<ImportedAssets> assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i);

public:
	static std::shared_ptr<AssetManager> singleton;

	std::shared_ptr<ImportedAssets> loadAssets(const std::string& filename);

};