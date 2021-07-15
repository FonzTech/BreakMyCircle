#include "OverlayGui.h"

#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "../Common/CommonUtility.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> OverlayGui::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayGui> p = std::make_shared<OverlayGui>(parent);
	return p;
}

OverlayGui::OverlayGui(const Int parentIndex) : GameObject(parentIndex)
{
	// Get assets
	Resource<GL::Mesh> mesh = getMesh();
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shader = CommonUtility::singleton->getFlat3DShader();
	Resource<GL::Texture2D> texture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_BUBBLE_BLUE);

	// Create drawable
	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;

	std::shared_ptr<TexturedDrawable<Shaders::Flat3D>> td = std::make_shared<TexturedDrawable<Shaders::Flat3D>>(*drawables, shader, mesh, texture);
	td->setParent(mManipulator.get());
	td->setDrawCallback(this);
	mDrawables.emplace_back(td);
}

const Int OverlayGui::getType() const
{
	return GOT_OVERLAY_GUI;
}

void OverlayGui::update()
{
	mDrawables[0]->setTransformation(Matrix4::scaling(Vector3(0.5f)));
}

void OverlayGui::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Flat3D&) baseDrawable->getShader())
		.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
		.bindTexture(*baseDrawable->mTexture)
		.draw(*baseDrawable->mMesh);
}

void OverlayGui::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void OverlayGui::setPosition(const Vector2 & position)
{
	mPosition = Vector3(position, 0.0f);
}

void OverlayGui::setSize(const Vector2 & size)
{
	mSize = size;
}

Resource<GL::Mesh> & OverlayGui::getMesh()
{
	// Get required resource
	Resource<GL::Mesh> resMesh{ CommonUtility::singleton->manager.get<GL::Mesh>(RESOURCE_MESH_PLANE_FLAT) };

	if (!resMesh)
	{
		// Create flat plane
		Trade::MeshData plane = Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates);

		GL::Buffer vertices;
		vertices.setData(MeshTools::interleave(plane.positions3DAsArray(), plane.textureCoordinates2DAsArray()));

		GL::Mesh mesh;
		mesh.setPrimitive(plane.primitive())
			.setCount(plane.vertexCount())
			.addVertexBuffer(std::move(vertices), 0, Shaders::Flat3D::Position{}, Shaders::Flat3D::TextureCoordinates{});

		// Add to resources
		CommonUtility::singleton->manager.set(resMesh.key(), std::move(mesh));
	}

	return resMesh;
}