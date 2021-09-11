#include "OverlayGui.h"

#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/Shaders/Flat.h>

#include "../Common/CommonUtility.h"
#include "../Graphics/GameDrawable.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> OverlayGui::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayGui> p = std::make_shared<OverlayGui>(parent, RESOURCE_TEXTURE_GUI_SETTINGS);
	return p;
}

OverlayGui::OverlayGui(const Int parentIndex) : AbstractGuiElement(parentIndex), mRotation(Deg(0.0f))
{
}

OverlayGui::OverlayGui(const Int parentIndex, const std::string & textureName) : OverlayGui(parentIndex)
{
	// Assign member
	mParentIndex = parentIndex;

	// Get assets
	Resource<GL::Mesh> mesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shader = CommonUtility::singleton->getFlat3DShader();
	Resource<GL::Texture2D> texture = CommonUtility::singleton->loadTexture(textureName);

	// Create drawable
	auto& drawables = RoomManager::singleton->mGoLayers[parentIndex].drawables;

	const std::shared_ptr<GameDrawable<Shaders::Flat3D>> td = std::static_pointer_cast<GameDrawable<Shaders::Flat3D>>(std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, shader, mesh, texture));
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
	updateAspectRatioFactors();
	updateTransformations();
}

void OverlayGui::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (Math::intersects(mBbox, outerFrame))
	{
		((Shaders::Flat3D&) baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(mColor)
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
}

void OverlayGui::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void OverlayGui::setRotationInDegrees(const Float rotation)
{
	mRotation = Rad(Deg(rotation));
}

void OverlayGui::setColor(const Color4 & color)
{
	mColor = color;
}

Range3D OverlayGui::getBoundingBox(const Vector2 & windowSize)
{
	const auto& s = Vector3(windowSize, 0.25f);
	const auto& o = s * 0.5f;
	return Range3D{
		{ mBbox.min().x() * s.x() + o.x(), s.y() - (mBbox.max().y() * s.y() + o.y()), mBbox.min().z() * s.z() + o.z() },
		{ mBbox.max().x() * s.x() + o.x(), s.y() - (mBbox.min().y() * s.y() + o.y()), mBbox.max().z() * s.z() + o.z() }
	};
}

const Resource<GL::Texture2D> & OverlayGui::getTextureResource() const
{
	return mDrawables[0]->mTexture;
}

const void OverlayGui::setTexture(const std::string & textureName)
{
	mDrawables[0]->mTexture = CommonUtility::singleton->loadTexture(textureName);
}

Float* OverlayGui::color()
{
	return mColor.data();
}

const Vector2 OverlayGui::getSize() const
{
	return mSize;
}

void OverlayGui::updateTransformations()
{
	const Float ar = mAspectRatio.aspectRatio();
	const Vector2 size = mSize / 1.77f * ar;

	Vector2 tp(mPosition.xy());
	tp += mAnchor * size / Vector2(1.0f, ar);

	Vector2 ts = mAspectRatio * size;

	(*mManipulator)
		.resetTransformation()
		.rotate(mRotation, Vector3::zAxis())
		.scale(Vector3(ts.x(), ts.y(), 1.0f))
		.translate(Vector3(tp.x(), tp.y(), 0.0f));

	const Range2D r = {
		tp - ts,
		tp + ts
	};
	mBbox = Range3D{
		{ r.min().x(), r.min().y(), -1.0f },
		{ r.max().x(), r.max().y(), 1.0f }
	};
}