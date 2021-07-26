#pragma once

#include <memory>
#include <nlohmann/json.hpp>

#include <Magnum/Text/Text.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/DistanceFieldGlyphCache.h>
#include <Magnum/Text/Renderer.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/DistanceFieldVector.h>
#include "../GameObject.h"

using namespace Magnum;

class OverlayText : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	OverlayText(const Int parentIndex);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	void setText(const std::string & text);

	Color4 mColor;
	Color4 mOutlineColor;
	Vector2 mOutlineRange;

protected:
	void updateTransformation();
	Resource<GL::AbstractShaderProgram, Shaders::DistanceFieldVector2D> getShader();

	Text::DistanceFieldGlyphCache mCache;
	Text::AbstractFont* mFont;
	Containers::Pointer<Text::Renderer2D> mText;

	Resource<GL::AbstractShaderProgram, Shaders::DistanceFieldVector2D> mShader;

	Vector2i mCurrentWindowSize;
	Vector2 mCurrentFloatWindowSize;
	Matrix3 mProjectionMatrix;
	Matrix3 mTransformationMatrix;
};