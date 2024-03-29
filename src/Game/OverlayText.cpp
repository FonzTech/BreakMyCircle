#include "OverlayText.h"

#include <Magnum/Math/Tags.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
#include <Magnum/GL/Renderer.h>

#include "../RoomManager.h"
#include "../InputManager.h"
#include "../Graphics/GameDrawable.h"

std::shared_ptr<GameObject> OverlayText::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayText> p = std::make_shared<OverlayText>(parent, Text::Alignment::MiddleCenter, 40);
	return p;
}

OverlayText::OverlayText(const Int parentIndex, const Text::Alignment & textAlignment, const UnsignedInt textCapacity) : AbstractGuiElement(parentIndex)
{
	// Init members
	mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
	mOutlineRange = Vector2(0.5f, 0.3f);

	// Load assets
	mFontHolder = CommonUtility::singleton->loadFont(RESOURCE_FONT_UBUNTU_TITLE);

	mText.reset(new Text::Renderer2D(*mFontHolder->font, *mFontHolder->cache, 32.0f, textAlignment));
	mText->reserve(textCapacity, GL::BufferUsage::DynamicDraw, GL::BufferUsage::StaticDraw);

	// Create dummy drawable
	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	std::shared_ptr<GameDrawable<Shaders::DistanceFieldVector2D>> td = std::make_shared<GameDrawable<Shaders::DistanceFieldVector2D>>(*drawables, getShader());
	td->setParent(mManipulator);
	td->setDrawCallback(this);
	mDrawables.emplace_back(td);
}

const Int OverlayText::getType() const
{
	return GOT_OVERLAY_TEXT;
}

void OverlayText::update()
{
	updateAspectRatioFactors();
	updateTransformations();
}

void OverlayText::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mColor.a() > 0.001f || mOutlineColor.a() > 0.001f)
	{
		if (Math::intersects(mBbox, outerFrame) && Math::abs(mSize.length()) > 0.0001f)
		{
			((Shaders::DistanceFieldVector2D&)baseDrawable->getShader())
				.bindVectorTexture(mFontHolder->cache->texture())
				.setTransformationProjectionMatrix(mProjectionMatrix * mTransformationMatrix)
				.setColor(mColor)
				.setOutlineColor(mOutlineColor)
				.setOutlineRange(mOutlineRange.x(), mOutlineRange.y())
				.setSmoothness(0.025f / mTransformationMatrix.uniformScaling())
				.draw(mText->mesh());
		}
	}
}

void OverlayText::drawDetached()
{
	draw(mDrawables.at(0).get(), Matrix4(Magnum::NoInit), *RoomManager::singleton->mCamera);
}

void OverlayText::setText(const std::string & text)
{
	mText->render(text);

	Int xct = 0;
	Int xcf = 0;
	Int yc = 1;

	for (auto it = text.begin(); true; ++it)
	{
		if (it == text.end())
		{
			xcf = Math::max(xcf, xct);
			break;
		}
		else if (*it == '\n')
		{
			++yc;
			xcf = Math::max(xcf, xct);
			xct = 0;
		}
		else
		{
			++xct;
		}
	}

	mTextSize = Vector2(Float(xcf), Float(yc));
}

Range3D OverlayText::getBoundingBox(const Vector2 & windowSize)
{
	const auto& s = Vector3(windowSize, 0.25f);
	const auto& o = s * 0.5f;
	return Range3D{
		{ mBbox.min().x() * s.x() + o.x(), s.y() - (mBbox.max().y() * s.y() + o.y()), mBbox.min().z() * s.z() + o.z() },
		{ mBbox.max().x() * s.x() + o.x(), s.y() - (mBbox.min().y() * s.y() + o.y()), mBbox.max().z() * s.z() + o.z() }
	};
}

void OverlayText::updateTransformations()
{
	const Vector2 ar = mAspectRatio > 1.0f ? Vector2(mAspectRatio, 1.0f) : Vector2(1.0f, 1.0f / mAspectRatio);
	const Vector2 s1 = mSize * mTextSize * ar;
	const Vector2 s2 = s1 * (0.02f / ar) * Vector2(1.25f, 2.25f);

	{
		Vector2 ws;
		Float scaleFactor;

		if (mCustomCanvasSize.x() >= 0.0f)
		{
			if (mCustomCanvasSize.x() >= 1.0f)
			{
				ws = mCustomCanvasSize;
				scaleFactor = 1.0f;
			}
			else
			{
				ws = CommonUtility::singleton->mFramebufferSize;
				scaleFactor = Math::min(1.0f, CommonUtility::singleton->mFramebufferSize.x() / 432.0f);
				scaleFactor *= CommonUtility::singleton->mConfig.displayDensity;
			}

			mProjectionMatrix = Matrix3::projection(ws);
		}
		else
		{
			ws = mCustomCanvasSize;
			mProjectionMatrix = Matrix3();
			scaleFactor = 1.0f;
		}


		const auto& tp = mPosition.xy() * ws + mAnchor * ws * ar * 0.5f;
		mTransformationMatrix = Matrix3::translation(tp) * Matrix3::scaling(mSize * scaleFactor);
	}

	const Range2D r = {
		mPosition.xy() - s2,
		mPosition.xy() + s2
	};
	mBbox = getTransformedBbox(r);
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