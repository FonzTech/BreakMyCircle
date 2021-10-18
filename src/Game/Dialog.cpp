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

Dialog::Dialog(const Int parentIndex, const UnsignedInt messageCapacity, const UnsignedInt titleCapacity) : GameObject(parentIndex), mMode(GO_DG_MODE_ACTIONS), mOpened(1.0f), mOpacity(0.0f), mRotation(0.0f), mClickIndex(-1)
{
	// Assign members
	mParentIndex = parentIndex;

	// Create background
	{
		mBackground = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_WHITE);
		mBackground->setColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		mBackground->setPosition({ 0.0f, 0.0f });
		mBackground->setAnchor({ 0.0f, 0.0f });
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(mBackground);
	}

	// Create title
	if (titleCapacity != 0U)
	{
		mTitle = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, titleCapacity);
		mTitle->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		mTitle->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		mTitle->mPosition = Vector3(0.0f, 0.4f, 0.0f);
		mTitle->setSize(Vector2(1.0f));
		mTitle->setText("---");
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(mTitle);
	}

	// Create message
	{
		mMessage = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, messageCapacity);
		mMessage->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
		mMessage->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 0.0f);
		mMessage->mPosition = Vector3(0.0f, 0.3f, 0.0f);
		mMessage->setSize(Vector2(1.0f));
		mMessage->setText("---");
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(mMessage);
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

	// Destroy loader
	if (mLoading != nullptr)
	{
		mLoading->mDestroyMe = true;
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
	// Set background size
	mBackground->setSize({ 1.0f * Math::max(1.0f, RoomManager::singleton->getWindowAspectRatio()), 1.0f });

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
	switch (mMode)
	{
	case GO_DG_MODE_ACTIONS:
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
		break;

	case GO_DG_MODE_LOADING:
		if (mLoading != nullptr)
		{
			mRotation += mDeltaTime * 90.0f;
			mLoading->setRotationInDegrees(mRotation);
		}
		break;
	}
}

void Dialog::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
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

void Dialog::addAction(const std::string & text, const std::function<void(UnsignedInt)> & callback, const bool isLong, const Vector3 & offset, const UnsignedInt capacity)
{
	const Float index(Float(mActions.size()) + 1.0f);
	const Float yp = -0.125f * index;

	const std::shared_ptr<OverlayGui> buttonGui = std::make_shared<OverlayGui>(mParentIndex, isLong ? RESOURCE_TEXTURE_GUI_BUTTON_4X1 : RESOURCE_TEXTURE_GUI_BUTTON_2X1);
	buttonGui->setPosition(Vector2(0.0f, yp) + offset.xy());
	buttonGui->setSize({ isLong ? 0.36f : 0.18f, 0.09f });
	buttonGui->setAnchor({ 0.0f, 0.0f });
	RoomManager::singleton->mGoLayers[mParentIndex].push_back(buttonGui);

	const std::shared_ptr<OverlayText> buttonText = std::make_shared<OverlayText>(mParentIndex, Text::Alignment::MiddleCenter, capacity != 0U ? capacity : UnsignedInt(text.length()));
	buttonText->mPosition = Vector3(0.0f, yp, 0.0f) + offset;
	buttonText->mColor = Color4(1.0f, 1.0f, 1.0f, 0.0f);
	buttonText->mOutlineColor = Color4(0.81f, 0.42f, 0.14f, 0.0f);
	buttonText->setSize(Vector2(0.8f));
	buttonText->setText(text);
	RoomManager::singleton->mGoLayers[mParentIndex].push_back(buttonText);

	mActions.push_back({
		callback,
		buttonGui,
		buttonText,
		0.0f
	});
}

void Dialog::setActionText(const UnsignedInt index, const std::string & text)
{
	mActions[index].buttonText->setText(text);
}

void Dialog::closeDialog()
{
	mOpened = -1.0f;
}

void Dialog::shakeButton(const UnsignedInt index)
{
	mActions[index].shake = 1.0f;
}

void Dialog::setMode(const Int mode)
{
	mMode = mode;
	switch (mode)
	{
	case GO_DG_MODE_ACTIONS:
		if (mLoading != nullptr)
		{
			mLoading->mDestroyMe = true;
			mLoading = nullptr;
		}
		break;

	case GO_DG_MODE_LOADING:
		{
			mLoading = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_GUI_LOADING);
			mLoading->setPosition({ 0.0f, -0.25f });
			mLoading->setSize(Vector2(0.25f));
			mLoading->setAnchor(Vector2(0.0f));
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(mLoading);
		}

		for (auto& action : mActions)
		{
			action.buttonGui->setAnchor(Vector2(100.0f));
			action.buttonText->setAnchor(Vector2(100.0f));
		}
		break;

	default:
		Error{} << "Unknown mode" << mode << "for Dialog";
	}
}