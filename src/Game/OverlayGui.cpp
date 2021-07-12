#include "OverlayGui.h"

#include "../Common/CommonUtility.h"
#include "Bubble.h"

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
	// Create game bubble
	CommonUtility::singleton->createGameSphere(this, *mManipulator, BUBBLE_COLOR_RED);
	mDrawables[0]->scale(Vector3(0.5f));
}

const Int OverlayGui::getType() const
{
	return GOT_OVERLAY_GUI;
}

void OverlayGui::update()
{
}

void OverlayGui::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(mPosition + Vector3(0.0f, 0.0f, 1.0f))
		.setLightColor(0x808080_rgbf)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0xc0c0c0_rgbf)
		.setDiffuseColor(0x808080_rgbf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void OverlayGui::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}