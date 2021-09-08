#include "Dialog.h"
#include "../RoomManager.h"
#include "../InputManager.h"
#include "../Common/CommonTypes.h"

#include <Magnum/Animation/Easing.h>

std::shared_ptr<GameObject> Dialog::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate bubble
	std::shared_ptr<Dialog> p = std::make_shared<Dialog>(parent);
	return p;
}

Dialog::Dialog(const Int parentIndex, const UnsignedInt messageCapacity, const UnsignedInt titleCapacity) : GameObject(parentIndex), mOpened(1.0f), mOpacity(0.0f), mClickIndex(-1)
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

	// Create title
	if (titleCapacity != 0U)
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, titleCapacity);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		go->mPosition = Vector3(0.0f, 0.4f, 0.0f);
		go->setSize(Vector2(1.0f));
		go->setText("---");

		mTitle = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
	}

	// Create message
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, messageCapacity);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		go->mPosition = Vector3(0.0f, 0.3f, 0.0f);
		go->setSize(Vector2(1.0f));
		go->setText("---");

		mMessage = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(go, true);
	}
}

Dialog::~Dialog()
{
	// Destroy all the mandatory GUI objects
	mBackground->mDestroyMe = true;

	if (mTitle != nullptr)
	{
		mTitle->mDestroyMe = true;
	}

	if (mMessage != nullptr)
	{
		mMessage->mDestroyMe = true;
	}

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
	const auto& lbs = InputManager::singleton->mMouseStates[PRIMARY_BUTTON];
	const Vector3 p(Float(InputManager::singleton->mMousePosition.x()), Float(InputManager::singleton->mMousePosition.y()), 0.0f);
	const auto& w = RoomManager::singleton->getWindowSize();

	const bool& clickable = mOpacity >= 1.0f;

	// Set background color
	mBackground->color()[3] = mOpacity * 0.8f;

	// Set title and message color
	if (mTitle != nullptr)
	{
		mTitle->mColor.data()[3] = mOpacity;
		mTitle->mOutlineColor.data()[3] = mOpacity;
	}

	if (mMessage != nullptr)
	{
		mMessage->mColor.data()[3] = mOpacity;
		mMessage->mOutlineColor.data()[3] = mOpacity;
	}

	// Set parameters for all actions
	for (UnsignedInt i = 0; i < mActions.size(); ++i)
	{
		// Get references
		auto& sf = mActions[i].shake;
		auto& ct = mActions[i].buttonText;
		auto& cg = mActions[i].buttonGui;

		// Set shake animation
		sf -= mDeltaTime;
		if (sf <= 0.0f)
		{
			sf = 0.0f;
		}

		{
			const Float sv = sf * 2.0f;
			const Float dx = sv - Math::floor(sv);
			const Float dy = dx > 0.5f ? (1.0f - dx) * 2.0f : (dx * 2.0f);
			const Float mult = sf > 0.5f ? -1.0f : 1.0f;
			const Vector2 sp = { Math::lerp(0.0f, 1.0f, Animation::Easing::quadraticOut(dy)) * mult, 0.0f };
			ct->setAnchor(sp);
			cg->setAnchor(sp);
		}

		// Set text opacity for
		ct->mColor.data()[3] = mOpacity;
		ct->mOutlineColor.data()[3] = mOpacity;

		// Set GUI opacity color
		cg->setColor(mActions[i].buttonText->mColor);

		// Check for click
		if (clickable)
		{
			const auto& b = cg->getBoundingBox(w);
			if (b.contains(p))
			{
				if (lbs == IM_STATE_PRESSED)
				{
					mClickIndex = i;
					break;
				}
				else if (lbs == IM_STATE_RELEASED && mClickIndex == i)
				{
					mActions[i].callback(i);
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

std::shared_ptr<OverlayText>& Dialog::getTitleDrawable()
{
	return mTitle;
}

std::shared_ptr<OverlayText>& Dialog::getMessageDrawable()
{
	return mMessage;
}

void Dialog::setTitle(const std::string & text)
{
	mTitle->setText(text);
}

void Dialog::setMessage(const std::string & text)
{
	mMessage->setText(text);
}

void Dialog::setTitlePosition(const Vector3 & position)
{
	mTitle->setPosition(position.xy());
}

void Dialog::setMessagePosition(const Vector3 & position)
{
	mMessage->setPosition(position.xy());
}

void Dialog::addAction(const std::string & text, const std::function<void(UnsignedInt)> & callback, const bool isLong, const Vector3 & offset)
{
	const Float index(Float(mActions.size()) + 1.0f);
	const Float yp = -0.125f * index;

	const std::shared_ptr<OverlayGui> buttonGui = std::make_shared<OverlayGui>(mParentIndex, isLong ? RESOURCE_TEXTURE_GUI_BUTTON_4X1 : RESOURCE_TEXTURE_GUI_BUTTON_2X1);
	buttonGui->setPosition(Vector2(0.0f, yp) + offset.xy());
	buttonGui->setSize({ isLong ? 0.36f : 0.18f, 0.09f });
	buttonGui->setAnchor({ 0.0f, 0.0f });

	const std::shared_ptr<OverlayText> buttonText = std::make_shared<OverlayText>(mParentIndex, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
	buttonText->mPosition = Vector3(0.0f, yp, 0.0f) + offset;
	buttonText->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
	buttonText->mOutlineColor = Color4(0.81f, 0.42f, 0.14f, 0.0f);
	buttonText->setSize(Vector2(0.8f));
	buttonText->setText(text);

	mActions.push_back({
		callback,
		(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(buttonGui, true),
		(std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[mParentIndex].push_back(buttonText, true),
		0.0f
	});
}

void Dialog::closeDialog()
{
	mOpened = -1.0f;
}

void Dialog::shakeButton(const UnsignedInt index)
{
	mActions[index].shake = 1.0f;
}