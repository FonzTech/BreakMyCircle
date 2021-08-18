#include "OverlayGuiDetached.h"

#include "../Common/CommonUtility.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> OverlayGuiDetached::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayGui> p = std::make_shared<OverlayGui>(parent, RESOURCE_TEXTURE_GUI_SETTINGS);
	return p;
}

OverlayGuiDetached::OverlayGuiDetached(const Int parentIndex, const std::string & textureName) : OverlayGui(parentIndex, textureName)
{
	// Assign member
	mParentIndex = parentIndex;

	// Get assets
	mMesh = CommonUtility::singleton->getPlaneMeshForFlatShader();
	mShader = CommonUtility::singleton->getFlat3DShader();
	mTexture = CommonUtility::singleton->loadTexture(textureName);
}

const Int OverlayGuiDetached::getType() const
{
	return GOT_OVERLAY_GUI_DETACHED;
}

void OverlayGuiDetached::update()
{
	// Update transformation
	OverlayGui::update();
	
	// Update projection
	const auto& w = RoomManager::singleton->getWindowSize();
	if (mCurrentWindowSize != w)
	{
		mCurrentWindowSize = w;
		mProjectionMatrix = Matrix4::orthographicProjection(w, 0.01f, 100.0f);
	}
}

void OverlayGuiDetached::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	drawDetached();
}

void OverlayGuiDetached::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void OverlayGuiDetached::drawDetached()
{
	(*mShader)
		.setTransformationProjectionMatrix(mProjectionMatrix * mManipulator->transformation())
		.bindTexture(*mTexture)
		.setColor(mColor)
		.setAlphaMask(0.001f)
		.draw(*mMesh);
}