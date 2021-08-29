#include "Dialog.h"
#include "../RoomManager.h"
#include "../InputManager.h"

std::shared_ptr<GameObject> Dialog::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate bubble
	std::shared_ptr<Dialog> p = std::make_shared<Dialog>(parent);
	return p;
}

Dialog::Dialog(const Int parentIndex, const UnsignedInt textCapacity) : GameObject(parentIndex), mOpened(1.0f), mOpacity(0.0f), mClickIndex(-1)
{
	// Assign members
	mParentIndex = parentIndex;

	// Create background
	{
		const std::shared_ptr<OverlayGui> go = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_WHITE);
		go->setColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		go->setPosition({ 0.0f, 0.0f });
		go->setSize({ 1.0f, 1.0f });
		go->setAnchor({ 0.0f, 0.0f });

		mBackground = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
	}

	// Create text
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::LineCenter, textCapacity);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		go->mPosition = Vector3(0.0f, 0.3f, 0.0f);
		go->setSize(Vector2(1.0f));
		go->setText("---");

		mText = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
	}
}

Dialog::~Dialog()
{
	// Destroy all the mandatory GUI objects
	mBackground->mDestroyMe = true;
	mText->mDestroyMe = true;

	// Destroy all of the added actions
	for (auto& action : mActions)
	{
		action.buttonGui->mDestroyMe = true;
		action.buttonText->mDestroyMe = true;
	}
}

const Int Dialog::getType() const
{
	return GOT_DIALOG;
}

void Dialog::update()
{
	// Control dialog life-cycle
	mOpacity += mDeltaTime * 2.0f * mOpened;
	if (mOpacity < 0.0f)
	{
		mDestroyMe = true;
	}
	else if (mOpacity > 1.0f)
	{
		mOpacity = 1.0f;
	}

	// Prepare work variables
	const auto& lbs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];
	const Vector3 p(Float(InputManager::singleton->mMousePosition.x()), Float(InputManager::singleton->mMousePosition.y()), 0.0f);
	const auto& w = RoomManager::singleton->getWindowSize();

	const bool& clickable = mOpacity >= 1.0f;

	// Set background color
	mBackground->color()[3] = mOpacity * 0.8f;

	// Set message color
	mText->mColor.data()[3] = mOpacity;
	mText->mOutlineColor.data()[3] = mOpacity;

	// Set parameters for all actions
	for (Int i = 0; i < mActions.size(); ++i)
	{
		// Set text opacity for
		mActions[i].buttonText->mColor.data()[3] = mOpacity;
		mActions[i].buttonText->mOutlineColor.data()[3] = mOpacity;

		// Set GUI opacity color
		mActions[i].buttonGui->setColor(mActions[i].buttonText->mColor);

		// Check for click
		if (clickable)
		{
			const auto& b = mActions[i].buttonGui->getBoundingBox(w);
			if (b.contains(p))
			{
				if (lbs == IM_STATE_PRESSED)
				{
					mClickIndex = i;
					break;
				}
				else if (lbs == IM_STATE_RELEASED && mClickIndex == i)
				{
					mActions[i].callback();
					break;
				}
			}
		}
	}
}

void Dialog::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
}

void Dialog::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Dialog::setMessage(const std::string & text)
{
	mText->setText(text);
}

void Dialog::setTextPosition(const Vector3 & position)
{
	mText->setPosition(position.xy());
}

void Dialog::addAction(const std::string & text, const std::function<void()> & callback, const Vector3 & offset)
{
	const Float index(Float(mActions.size()) + 1.0f);
	const Float yp = -0.15f * index;

	const std::shared_ptr<OverlayGui> buttonGui = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_GUI_BUTTON_2X1);
	buttonGui->setPosition(Vector2(0.0f, yp) + offset.xy());
	buttonGui->setSize({ 0.2f, 0.1f });
	buttonGui->setAnchor({ 0.0f, 0.0f });

	const std::shared_ptr<OverlayText> buttonText = std::make_shared<OverlayText>(mParentIndex, Text::Alignment::MiddleCenter, 10);
	buttonText->mPosition = Vector3(0.0f, yp, 0.0f) + offset;
	buttonText->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
	buttonText->mOutlineColor = Color4(0.81f, 0.42f, 0.14f, 0.0f);
	buttonText->setSize(Vector2(1.0f));
	buttonText->setText(text);

	mActions.push_back({
		callback,
		(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(buttonGui, true),
		(std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(buttonText, true)
	});
}

void Dialog::closeDialog()
{
	mOpened = -1.0f;
}