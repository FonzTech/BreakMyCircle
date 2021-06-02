#include "CommonUtility.h"

#include <Magnum/Magnum.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Math.h>

#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

namespace CommonUtility
{
	std::shared_ptr<ColoredDrawable> createGameSphere(Object3D & parent, const Vector3 & ambientColor, IDrawCallback* drawCallback)
	{
		// Create test mesh
		Trade::MeshData meshData = Primitives::icosphereSolid(2U);

		GL::Buffer vertices;
		vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.normalsAsArray()));

		std::pair<Containers::Array<char>, MeshIndexType> compressed = MeshTools::compressIndices(meshData.indicesAsArray());
		GL::Buffer indices;
		indices.setData(compressed.first);

		std::shared_ptr<GL::Mesh> mesh = std::make_shared<GL::Mesh>();
		(*mesh.get())
			.setPrimitive(meshData.primitive())
			.setCount(meshData.indexCount())
			.addVertexBuffer(std::move(vertices), 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
			.setIndexBuffer(std::move(indices), 0, compressed.second);

		// Create Phong shader
		std::shared_ptr<Shaders::Phong> shader = std::make_shared<Shaders::Phong>();

		// Create colored drawable
		std::shared_ptr<ColoredDrawable> cd = std::make_shared<ColoredDrawable>(RoomManager::singleton->mDrawables, shader, mesh, 0xffffff_rgbf);
		cd->setParent(&parent);
		cd->setDrawCallback(drawCallback);
		return cd;
	}

	std::shared_ptr<ColoredDrawable> createPlane(Object3D & parent, IDrawCallback* drawCallback)
	{
		// Create test mesh
		Trade::MeshData meshData = Primitives::planeSolid();

		GL::Buffer vertices;
		vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.normalsAsArray()));

		std::shared_ptr<GL::Mesh> mesh = std::make_shared<GL::Mesh>();
		(*mesh.get())
			.setPrimitive(meshData.primitive())
			.setCount(meshData.vertexCount())
			.addVertexBuffer(vertices, 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{});

		// Create Phong shader
		std::shared_ptr<Shaders::Flat3D> shader = std::make_shared<Shaders::Flat3D>();

		// Create colored drawable
		std::shared_ptr<ColoredDrawable> cd = std::make_shared<ColoredDrawable>(RoomManager::singleton->mDrawables, shader, mesh, 0xffffff_rgbf);
		cd->setParent(&parent);
		cd->setDrawCallback(drawCallback);
		return cd;
	}
}