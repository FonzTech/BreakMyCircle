#include "CommonUtility.h"

#include <Corrade/Corrade.h>
#include <Corrade/Containers/PointerStl.h>
#include <Magnum/Magnum.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/GL/Mesh.h>
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

std::shared_ptr<ColoredDrawable<Shaders::Phong>> CommonUtility::createGameSphere(Object3D & parent, const Vector3 & ambientColor, IDrawCallback* drawCallback)
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
	std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = std::make_shared<ColoredDrawable<Shaders::Phong>>(RoomManager::singleton->mDrawables, resShader, resMesh, 0xffffff_rgbf);
	cd->setParent(&parent);
	cd->setDrawCallback(drawCallback);
	return cd;
}