#include "OverlayText.h"

#include <Magnum/Math/Tags.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/GL/Renderer.h>

#include "../RoomManager.h"
#include "../InputManager.h"

std::shared_ptr<GameObject> OverlayText::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayText> p = std::make_shared<OverlayText>(parent, Text::Alignment::MiddleCenter, 40);
	return p;
}

OverlayText::OverlayText(const Int parentIndex, const Text::Alignment & textAlignment, const UnsignedInt textCapacity) : GameObject(parentIndex), mScale{1.0f}
{
	// Init members
	mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
	mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOutlineRange = Vector2(0.5f, 0.3f);

	// Load assets
	mFontHolder = CommonUtility::singleton->loadFont(RESOURCE_FONT_UBUNTU_TITLE);

	mText.reset(new Text::Renderer2D(*mFontHolder->font, *mFontHolder->cache, 32.0f, textAlignment));
	mText->reserve(textCapacity, GL::BufferUsage::DynamicDraw, GL::BufferUsage::StaticDraw);

	// Create dummy drawable
	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	std::shared_ptr<GameDrawable<Shaders::DistanceFieldVector2D>> td = std::make_shared<GameDrawable<Shaders::DistanceFieldVector2D>>(*drawables, getShader());
	td->setParent(mManipulator.get());
	td->setDrawCallback(this);
	mDrawables.emplace_back(td);
}

const Int OverlayText::getType() const
{
	return GOT_OVERLAY_TEXT;
}

void OverlayText::update()
{
	const auto& w = RoomManager::singleton->getWindowSize();
	if (mCurrentWindowSize != w)
	{
		mCurrentWindowSize = w;
		mProjectionMatrix = Matrix3::projection(mCurrentWindowSize);
		updateTransformation();
	}
}

void OverlayText::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mColor.a() > 0.0f || mOutlineColor.a() > 0.0f)
	{
		((Shaders::DistanceFieldVector2D&)mDrawables.at(0)->getShader())
			.bindVectorTexture(mFontHolder->cache->texture())
			.setTransformationProjectionMatrix(mProjectionMatrix * mTransformationMatrix)
			.setColor(mColor)
			.setOutlineColor(mOutlineColor)
			.setOutlineRange(mOutlineRange.x(), mOutlineRange.y())
			.setSmoothness(0.025f / mTransformationMatrix.uniformScaling())
			.draw(mText->mesh());
	}
}

void OverlayText::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void OverlayText::drawDetached()
{
	draw(mDrawables.at(0).get(), Matrix4(Magnum::NoInit), *RoomManager::singleton->mCamera);
}

void OverlayText::setText(const std::string & text)
{
	mText->render(text);
}

void OverlayText::setPosition(const Vector3 & position)
{
	mPosition = position;
	updateTransformation();
}

void OverlayText::setScale(const Vector2 & scale)
{
	mScale = scale;
	updateTransformation();
}

void OverlayText::updateTransformation()
{
	mTransformationMatrix = Matrix3::translation(mCurrentWindowSize * mPosition.xy()) * Matrix3::scaling(mScale);
	/*
	(*mManipulator)
		.resetTransformation()
		.scale(mScale)
		.translate(mPosition);
	*/
}

Resource<GL::AbstractShaderProgram, Shaders::DistanceFieldVector2D> OverlayText::getShader()
{
	// Get required resource
	Resource<GL::AbstractShaderProgram, Shaders::DistanceFieldVector2D> resShader{ CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, Shaders::DistanceFieldVector2D>(RESOURCE_SHADER_DISTANCE_FIELD_VECTOR) };

	if (!resShader)
	{
		// Create shader
		std::unique_ptr<GL::AbstractShaderProgram> shader = std::make_unique<Shaders::DistanceFieldVector2D>();

		// Add to resources
		Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
		CommonUtility::singleton->manager.set(resShader.key(), std::move(p));
	}

	return resShader;
}