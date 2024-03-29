#pragma once

#include <memory>
#include <string>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "Common/CommonTypes.h"
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
public:
	AssetManager();
	AssetManager(const std::string & coloredShaderResourceKey, const std::string & texturedShaderResourceKey, const Int lightCount);

	void loadAssets(GameObject& gameObject, Object3D& manipulator, const std::string& filename, IDrawCallback* drawCallback);
	Resource<GL::AbstractShaderProgram, Shaders::Phong> getColoredShader(const std::string & resourceKey, const Int lightCount);
	Resource<GL::AbstractShaderProgram, Shaders::Phong> getTexturedShader(const std::string & resourceKey, const Int lightCount);

protected:
	Resource<GL::AbstractShaderProgram, Shaders::Phong> coloredShader;
	Resource<GL::AbstractShaderProgram, Shaders::Phong> texturedShader;

	void processChildrenAssets(GameObject& gameObject, ImportedAssets& assets, Trade::AbstractImporter& importer, Object3D& parent, UnsignedInt i, IDrawCallback* drawCallback);
};