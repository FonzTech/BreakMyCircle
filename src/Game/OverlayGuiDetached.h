#pragma once

#include "OverlayGui.h"
#include "../Graphics/IDrawDetached.h"

class OverlayGuiDetached : public OverlayGui, public IDrawDetached
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	OverlayGuiDetached(const Int parentIndex, const std::string & textureName);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;
	void drawDetached() override;

protected:

	// Resources to use for detached drawing
	Resource<GL::Mesh> mMesh;
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> mShader;
	Resource<GL::Texture2D> mTexture;

	// Members for projection calculation
	Vector2i mCurrentWindowSize;
	Vector2 mCurrentFloatWindowSize;

	// Projection matrix
	Matrix4 mProjectionMatrix;
};