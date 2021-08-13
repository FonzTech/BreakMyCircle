#include "OverlayText.h"

#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "../RoomManager.h"
#include "../InputManager.h"

std::shared_ptr<GameObject> OverlayText::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayText> p = std::make_shared<OverlayText>(parent, Text::Alignment::MiddleCenter);
	return p;
}

OverlayText::OverlayText(const Int parentIndex, const Text::Alignment & textAlignment) : GameObject(parentIndex), mScale{1.0f}, mCurrentWindowSize{ 0, 0 }
{
	// Init members
	mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
	mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOutlineRange = Vector2(0.5f, 0.3f);

	// Load assets
	mFontHolder = CommonUtility::singleton->loadFont(RESOURCE_FONT_UBUNTU_TITLE);

	mText.reset(new Text::Renderer2D(*mFontHolder->font, *mFontHolder->cache, 32.0f, textAlignment));
	mText->reserve(40, GL::BufferUsage::DynamicDraw, GL::BufferUsage::StaticDraw);

	mShader = getShader();
}

const Int OverlayText::getType() const
{
	return GOT_OVERLAY_TEXT;
}

void OverlayText::update()
{
	if (mCurrentWindowSize != RoomManager::singleton->mWindowSize)
	{
		mCurrentWindowSize = RoomManager::singleton->mWindowSize;
		mCurrentFloatWindowSize = Vector2{ mCurrentWindowSize };
		mProjectionMatrix = Matrix3::projection(mCurrentFloatWindowSize);
		updateTransformation();
	}
}

void OverlayText::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	drawDetached();
}

void OverlayText::drawDetached()
{
	if (mColor.a() > 0.0f || mOutlineColor.a() > 0.0f)
	{
		(*mShader)
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
	mTransformationMatrix = Matrix3::translation(mCurrentFloatWindowSize * mPosition.xy()) * Matrix3::scaling(mScale);
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