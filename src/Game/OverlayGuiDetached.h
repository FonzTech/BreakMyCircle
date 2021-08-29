#pragma once

#define GO_OGD_FLAT 1
#define GO_OGD_PLASMA 2

#include "OverlayGui.h"
#include "../Graphics/IDrawDetached.h"

class OverlayGuiDetached : public OverlayGui, public IDrawDetached
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	// Class members
	OverlayGuiDetached(const Int parentIndex, const std::string & textureName, const Int customType);

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;
	void drawDetached() override;
	
	GL::AbstractShaderProgram* getShader();

protected:

	// Resources to use for detached drawing
	Int mCustomType;
	Resource<GL::Mesh> mMesh;
	Resource<GL::Texture2D> mTexture;
	GL::AbstractShaderProgram* mShader;

	// Projection matrix
	Matrix4 mProjectionMatrix;
};