#include "Dialog.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> Dialog::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate bubble
	std::shared_ptr<Dialog> p = std::make_shared<Dialog>(parent);
	return p;
}

Dialog::Dialog(const Int parentIndex) : GameObject(parentIndex), mOpacity(0.0f)
{
	// Assign members
	mParentIndex = parentIndex;

	// Create background
	{
		const std::shared_ptr<OverlayGui> go = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_WHITE);
		go->setPosition({ 0.0f, 0.0f });
		go->setSize({ 1.0f, 1.0f });
		go->setAnchor({ 0.0f, 0.0f });

		mBackground = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
	}

	// Create text
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::TopCenter);
		go->mPosition = Vector3(0.0f, 0.35f, 0.0f);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		go->setScale(Vector2(1.0f));
		go->setText("---");

		mText = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
	}
}

const Int Dialog::getType() const
{
	return GOT_DIALOG;
}

void Dialog::update()
{
	// Control animation for dialog
	mOpacity += mDeltaTime * 2.0f;
	if (mOpacity > 1.0f)
	{
		mOpacity = 1.0f;
	}

	// Set background color
	mBackground->setColor(Color4(0.0f, 0.0f, 0.0f, mOpacity * 0.5f));

	mText->mColor = Color4(1.0f, 1.0f, 1.0f, mOpacity);
	mText->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, mOpacity);
}

void Dialog::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
}

void Dialog::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Dialog::setText(const std::string & text)
{
	mText->setText(text);
}