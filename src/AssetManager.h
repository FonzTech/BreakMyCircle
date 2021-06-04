#pragma once

#include <memory>
#include <string>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "CommonTypes.h"
#include "GameObject.h"

using namespace Magnum;

struct ImportedAssets
{
	Containers::Array<Resource<GL::Mesh>> meshes;
	Containers::Array<Resource<GL::Texture2D>> textures;
	Containers::Array<Resource<Trade::AbstractMaterialData>> materials;
};

class AssetManager
{
private:
	Resource<GL::AbstractShaderProgram, Shaders::Phong> coloredShader;
	Resource<GL::AbstractShaderProgram, Shaders::Phong> texturedShader;

	void processChildrenAssets(GameObject& gameObject, ImportedAssets& assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i, IDrawCallback* drawCallback);

public:
	static std::unique_ptr<AssetManager> singleton;

	AssetManager();

	void loadAssets(GameObject& gameObject, const std::string& filename, IDrawCallback* drawCallback);
};