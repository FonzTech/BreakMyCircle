#include "OverlayText.h"

#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "../Common/CommonUtility.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> OverlayText::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayText> p = std::make_shared<OverlayText>(parent);
	return p;
}

OverlayText::OverlayText(const Int parentIndex) : GameObject(parentIndex), mCache{ Vector2i{2048}, Vector2i{512}, 22 }, mCurrentWindowSize{ 0, 0 }
{
	// Init members
	mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
	mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOutlineRange = Vector2(0.45f, 0.35f);

	// Load assets
	mFont = CommonUtility::singleton->loadFont<Text::StbTrueTypeFont>(RESOURCE_FONT_UBUNTU_TITLE);

	mText.reset(new Text::Renderer2D(*mFont, mCache, 32.0f, Text::Alignment::TopRight));
	mText->reserve(40, GL::BufferUsage::DynamicDraw, GL::BufferUsage::StaticDraw);

	mShader = getShader();
}

const Int OverlayText::getType() const
{
	return GOT_OVERLAY_GUI;
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
	((Shaders::DistanceFieldVector2D&) mShader)
		.bindVectorTexture(mCache.texture())
		.setTransformationProjectionMatrix(mProjectionMatrix * mTransformationMatrix)
		.setColor(mColor)
		.setOutlineColor(mOutlineColor)
		.setOutlineRange(mOutlineRange.x(), mOutlineRange.y())
		.setSmoothness(0.025f / mTransformationMatrix.uniformScaling())
		.draw(mText->mesh());

}

void OverlayText::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void OverlayText::setText(const std::string & text)
{
	mText->render(text);
}

void OverlayText::updateTransformation()
{
	mTransformationMatrix = Matrix3::translation(mCurrentFloatWindowSize * mPosition.xy());
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