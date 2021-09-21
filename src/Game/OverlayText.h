#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include <Magnum/Text/Text.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/Renderer.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/DistanceFieldVector.h>

#include "../Game/AbstractGuiElement.h"
#include "../Common/CommonUtility.h"
#include "../Graphics/IDrawDetached.h"

using namespace Magnum;

class OverlayText : public AbstractGuiElement, public IDrawDetached
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	OverlayText(const Int parentIndex, const Text::Alignment & textAlignment, const UnsignedInt textCapacity);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void drawDetached() override;

	Range3D getBoundingBox(const Vector2 & windowSize) override;

	void setText(const std::string & text);

	Color4 mOutlineColor;
	Vector2 mOutlineRange;
	Vector2 mTextSize;

protected:
	void updateTransformations() override;
	Resource<GL::AbstractShaderProgram, Shaders::DistanceFieldVector2D> getShader();

	Resource<FontHolder> mFontHolder;
	Containers::Pointer<Text::Renderer2D> mText;

	Matrix3 mProjectionMatrix;
	Matrix3 mTransformationMatrix;
};