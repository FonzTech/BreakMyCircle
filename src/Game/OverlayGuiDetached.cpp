#include "OverlayGuiDetached.h"

#include "../Common/CommonUtility.h"
#include "../Shaders/PlasmaShader.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> OverlayGuiDetached::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<OverlayGuiDetached> p = std::make_shared<OverlayGuiDetached>(parent, RESOURCE_TEXTURE_GUI_SETTINGS, 0);
	return p;
}

OverlayGuiDetached::OverlayGuiDetached(const Int parentIndex, const std::string & textureName, const Int customType) : OverlayGui(parentIndex, textureName), mCustomType(customType)
{
	// Assign member
	mParentIndex = parentIndex;

	// Load assets
	switch (mCustomType)
	{
	case GO_OGD_FLAT:
		mShader = &*CommonUtility::singleton->getFlat3DShader();
		mMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D>(RESOURCE_MESH_PLANE_FLAT);
		mTexture = CommonUtility::singleton->loadTexture(textureName);
		break;

	case GO_OGD_PLASMA:
		mShader = &*CommonUtility::singleton->getPlasmaShader();
		mMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<PlasmaShader>(RESOURCE_MESH_PLANE_PLASMA);
		break;
	}
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
	switch (mCustomType)
	{
	case GO_OGD_FLAT:
		(*((Shaders::Flat3D*)mShader))
			.setTransformationProjectionMatrix(mProjectionMatrix * mManipulator->transformation())
			.bindTexture(*mTexture)
			.setColor(mColor)
			.setAlphaMask(0.001f)
			.draw(*mMesh);
		break;

	case GO_OGD_PLASMA:
		(*((PlasmaShader*)mShader))
			.setProjectionMatrix(mProjectionMatrix)
			.setTransformationMatrix(mManipulator->transformation())
			.draw(*mMesh);
		break;
	}
}

GL::AbstractShaderProgram* OverlayGuiDetached::getShader()
{
	return mShader;
}