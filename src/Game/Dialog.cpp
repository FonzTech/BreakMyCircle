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

Dialog::Dialog(const Int parentIndex) : GameObject(parentIndex), mOpened(1.0f), mOpacity(0.0f), mClickIndex(-1)
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
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::LineCenter);
		go->mPosition = Vector3(0.0f, 0.3f, 0.0f);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		go->setScale(Vector2(1.0f));
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
	const Vector2 w(Float(RoomManager::singleton->mWindowSize.x()), Float(RoomManager::singleton->mWindowSize.y()));

	const bool& clickable = mOpacity >= 1.0f;

	// Set background color
	mBackground->setColor(Color4(0.0f, 0.0f, 0.0f, mOpacity * 0.91f));

	// Set message color
	mText->mColor = Color4(1.0f, 1.0f, 1.0f, mOpacity);
	mText->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, mOpacity);

	// Set parameters for all actions
	for (Int i = 0; i < mActions.size(); ++i)
	{
		// Set text opacity for
		mActions[i].buttonText->mColor = Color4(1.0f, 1.0f, 1.0f, mOpacity);
		mActions[i].buttonText->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, mOpacity);

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

void Dialog::addAction(const std::string & text, const std::function<void()> & callback)
{
	const Float index(Float(mActions.size()) + 1.0f);
	const Float yp = -0.15f * index;

	const std::shared_ptr<OverlayGui> buttonGui = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_WHITE);
	buttonGui->setPosition({ 0.0f, yp });
	buttonGui->setSize({ 0.4f, 0.1f });
	buttonGui->setAnchor({ 0.0f, 0.0f });

	const std::shared_ptr<OverlayText> buttonText = std::make_shared<OverlayText>(mParentIndex, Text::Alignment::MiddleCenter);
	buttonText->mPosition = Vector3(0.0f, yp, 0.0f);
	buttonText->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
	buttonText->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
	buttonText->setScale(Vector2(1.0f));
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