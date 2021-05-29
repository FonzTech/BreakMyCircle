#include "CommonUtility.h"

#include <Magnum/Magnum.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Math.h>

#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

namespace CommonUtility
{
	std::shared_ptr<ColoredDrawable>createGameSphere(const Vector3 & ambientColor, IDrawCallback* drawCallback)
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

		// Create Phong shader
		Shaders::Phong shader;

		// Create colored drawable
		std::shared_ptr<ColoredDrawable> cd = std::make_shared<ColoredDrawable>(RoomManager::singleton->mDrawables, shader, mesh, 0xffffff_rgbf);
		cd->setParent(&RoomManager::singleton->mScene);
		cd->setDrawCallback(drawCallback);
		return cd;
	}
}