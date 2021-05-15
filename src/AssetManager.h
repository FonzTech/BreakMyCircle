#pragma once

#include <memory>
#include <string>
#include <Corrade/Containers/Array.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>

using namespace Magnum;

class AssetManager
{
protected:
	Containers::Array<Containers::Optional<GL::Mesh>> _meshes;
	Containers::Array<Containers::Optional<GL::Texture2D>> _textures;

public:
	static std::shared_ptr<AssetManager> singleton;

	void loadMesh(const std::string& filename);
};