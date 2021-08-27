#include "RoomManager.h"

#include <vector>
#include <unordered_set>
#include <Corrade/Utility/Directory.h>
#include <Magnum/Audio/AbstractImporter.h>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "Common/CommonUtility.h"
#include "Common/PerlinNoise.hpp"
#include "Game/Player.h"
#include "Game/Bubble.h"
#include "Game/Projectile.h"
#include "Game/FallingBubble.h"
#include "Game/Scenery.h"
#include "Game/Logo.h"
#include "Game/Skybox.h"
#include "Game/LevelSelector.h"
#include "Game/OverlayGui.h"
#include "Game/OverlayGuiDetached.h"
#include "Game/OverlayText.h"
#include "Game/LimitLine.h"

using namespace Magnum::Math::Literals;

std::unique_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::RoomManager() : mCurrentBoundParentIndex(-1), mSfxLevel(1.0f)
{
	// Create audio manager
	mAudioContext = std::make_unique<Audio::Context>(
		Audio::Context::Configuration{}
		.setHrtf(Audio::Context::Configuration::Hrtf::Enabled)
		.setFrequency(44100)
		.setRefreshRate(50)
	);
	mAudioListener = std::make_unique<Audio::Listener3D>(mScene);

	// Map every game object to this map
	gameObjectCreators[GOT_PLAYER] = Player::getInstance;
	gameObjectCreators[GOT_BUBBLE] = Bubble::getInstance;
	gameObjectCreators[GOT_PROJECTILE] = Projectile::getInstance;
	gameObjectCreators[GOT_FALLING_BUBBLE] = FallingBubble::getInstance;
	gameObjectCreators[GOT_SCENERY] = Scenery::getInstance;
	gameObjectCreators[GOT_LOGO] = Logo::getInstance;
	gameObjectCreators[GOT_SKYBOX] = Skybox::getInstance;
	gameObjectCreators[GOT_OVERLAY_GUI] = OverlayGui::getInstance;
	gameObjectCreators[GOT_OVERLAY_GUI_DETACHED] = OverlayGuiDetached::getInstance;
	gameObjectCreators[GOT_LEVEL_SELECTOR] = LevelSelector::getInstance;
	gameObjectCreators[GOT_OVERLAY_TEXT] = OverlayText::getInstance;
	gameObjectCreators[GOT_LIMIT_LINE] = LimitLine::getInstance;

	// Create collision manager
	mCollisionManager = std::make_unique<CollisionManager>();

	// Setup randomizer
	mBubbleColors[BUBBLE_COLOR_RED.toSrgbInt()] = { BUBBLE_COLOR_RED, RESOURCE_TEXTURE_BUBBLE_RED };
	mBubbleColors[BUBBLE_COLOR_GREEN.toSrgbInt()] = { BUBBLE_COLOR_GREEN, RESOURCE_TEXTURE_BUBBLE_GREEN };
	mBubbleColors[BUBBLE_COLOR_BLUE.toSrgbInt()] = { BUBBLE_COLOR_BLUE, RESOURCE_TEXTURE_BUBBLE_BLUE };
	mBubbleColors[BUBBLE_COLOR_YELLOW.toSrgbInt()] = { BUBBLE_COLOR_YELLOW, RESOURCE_TEXTURE_BUBBLE_YELLOW };
	mBubbleColors[BUBBLE_COLOR_PURPLE.toSrgbInt()] = { BUBBLE_COLOR_PURPLE, RESOURCE_TEXTURE_BUBBLE_PURPLE };
	mBubbleColors[BUBBLE_COLOR_ORANGE.toSrgbInt()] = { BUBBLE_COLOR_ORANGE, RESOURCE_TEXTURE_BUBBLE_ORANGE };
	mBubbleColors[BUBBLE_COLOR_CYAN.toSrgbInt()] = { BUBBLE_COLOR_CYAN, RESOURCE_TEXTURE_BUBBLE_CYAN };
	mBubbleColors[BUBBLE_COIN.toSrgbInt()] = { BUBBLE_COIN, RESOURCE_TEXTURE_BUBBLE_TRANSLUCENT };
	mBubbleColors[BUBBLE_BOMB.toSrgbInt()] = { BUBBLE_BOMB, RESOURCE_TEXTURE_BUBBLE_TRANSLUCENT };
	mBubbleColors[BUBBLE_PLASMA.toSrgbInt()] = { BUBBLE_PLASMA, RESOURCE_TEXTURE_BUBBLE_TRANSLUCENT };
	mBubbleColors[BUBBLE_ELECTRIC.toSrgbInt()] = { BUBBLE_ELECTRIC, RESOURCE_TEXTURE_WHITE };

	// Initialize game save data
	mSaveData.maxLevelId = 10U;
	mSaveData.coinTotal = 0;
	mSaveData.coinCurrent = 0;
	
	for (UnsignedInt i = 0; i < GO_LS_MAX_POWERUP_COUNT; ++i)
	{
#if NDEBUG or _DEBUG
		mSaveData.powerupAmounts[GO_LS_GUI_POWERUP + i] = 2;
#else
		mSaveData.powerupAmounts[GO_LS_GUI_POWERUP + i] = 0;
#endif
	}
}

RoomManager::~RoomManager()
{
	clear();
}

const Float RoomManager::getWindowAspectRatio() const
{
	return mAspectRatio;
}

const Vector2 RoomManager::getWindowSize()
{
	return mWindowSize;
}

const void RoomManager::setWindowSize(const Vector2 & size)
{
	mWindowSize = size;
	mAspectRatio = mWindowSize.aspectRatio();
}

const Int RoomManager::getCurrentBoundParentIndex() const
{
	return mCurrentBoundParentIndex;
}

const void RoomManager::setCurrentBoundParentIndex(const Int parentIndex)
{
	mCurrentBoundParentIndex = parentIndex;
}

const Float RoomManager::getBgMusicGain() const
{
	return mBgMusic->playable()->gain();
}

const void RoomManager::setBgMusicGain(const Float level)
{
	mBgMusic->playable()->setGain(level);
}

const Float RoomManager::getSfxGain() const
{
	return mSfxLevel;
}

const void RoomManager::setSfxGain(const Float level)
{
	mSfxLevel = level;
}

void RoomManager::clear()
{
	// Clear all layers and their children
	mGoLayers.clear();

	// Clear audio context
	if (mBgMusic != nullptr)
	{
		mBgMusic->clear();
		mBgMusic = nullptr;
	}
	mAudioContext = nullptr;

	// De-reference camera
	mCamera = nullptr;
}

void RoomManager::setup()
{
	// Set parent for camera
	mCameraObject.setParent(&mScene);

	// Setup camera
	mCamera = std::make_shared<SceneGraph::Camera3D>(mCameraObject);
	mCamera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::NotPreserved);
	mCamera->setViewport(GL::defaultFramebuffer.viewport().size());
}

void RoomManager::prepareRoom(const bool stopBgMusic)
{
	// Delete all game objects across all layers
	for (const auto& layer : mGoLayers)
	{
		layer.second.list->clear();
	}

	// Delete background music
	if (stopBgMusic)
	{
		mBgMusic->clear();
		mBgMusic = nullptr;
	}
}

void RoomManager::loadRoom(const std::string & name)
{
	// Load room from file
	const auto& content = Utility::Directory::readString("rooms/" + name + ".txt");
	const auto& roomData = nlohmann::json::parse(content);

	// Load audio
	{
		const auto& it = roomData.find("bgmusic");
		if (it != roomData.end())
		{
			std::string bgmusic = it->get<std::string>();

			mBgMusic = std::make_unique<StreamedAudioPlayable>(&mCameraObject);
			mBgMusic->loadAudio(bgmusic);
			mBgMusic->playable()->setGain(0.25f);
		}
	}

	// Iterate through list
	const std::vector<nlohmann::json> & list = roomData["objects"].get<std::vector<nlohmann::json>>();
	for (const auto& item : list)
	{
		// Get type
		Int parent, type;
		item.at("parent").get_to(parent);
		item.at("type").get_to(type);

		// Check for type
		std::shared_ptr<GameObject> gameObject = nullptr;
		{
			const auto& it = gameObjectCreators.find(type);
			if (it == gameObjectCreators.end())
			{
				printf("Could not find instantiator function for type %u. Skipping it.\n", type);
				continue;
			}

			const auto& fx = it->second;
			gameObject = fx(item);
		}

		// Push into room
		mGoLayers[parent].push_back(gameObject);
	}
}

void RoomManager::createLevelRoom(const std::shared_ptr<IShootCallback> & shootCallback, const Int xlen, const Int ylen, const int32_t difficulty)
{
	// Delete game level layer
	mGoLayers[GOL_PERSP_SECOND].list->clear();

	// Create bubbles
	const siv::PerlinNoise perlin(mSeed);

	const Float fSquare(xlen);

	for (Int i = 0; i < ylen; ++i)
	{
		for (Int j = 0; j < xlen; ++j)
		{
			// Working variables
			Float y = (Float) i;
			Float x = (Float) j;

			// Work with noise value to get the actual in-game object
			std::unique_ptr<Instantiator> pi = nullptr;
			{
				// Get noise value at this position
				double dx(1.0f / fSquare * x);
				double dy(1.0f / fSquare * y);

				// Get valid instantiator
				while (true)
				{
					const double value = dy > 0.01 ? perlin.accumulatedOctaveNoise2D_0_1(dx, dy, difficulty) : 1.0;
					pi = getGameObjectFromNoiseValue(value);
					if (pi != nullptr)
					{
						break;
					}

					dx += 0.01;
					dy += 0.01;

					dx -= Math::floor(dx);
					dy -= Math::floor(dy);
				}
			}

			const auto &it = gameObjectCreators.find(pi->key);
			if (it == gameObjectCreators.end())
			{
				printf("Could not find instantiator function for type %u. Skipping it.\n", pi->key);
				continue;
			}

			const auto& fx = it->second;
			const auto& gameObject = fx(*pi->params);

			// Get position for object
			Float startX;
			if (i % 2)
			{
				if (j == xlen - 1)
				{
					continue;
				}
				startX = 2.0f;
			}
			else
			{
				startX = 1.0f;
			}

			Vector3 position = { startX + x * 2.0f, y * -2.0f, 0.0f };

			gameObject->mPosition = position;
			RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(gameObject);
		}
	}

	// Setup projectile parameters
	const Float len = fSquare * 2.0f; // "2" is the fixed diameter of a "game bubble"
	{
		Projectile::setGlobalParameters(1.0f, len - 1.0f);
	}

	// Create player
	std::shared_ptr<GameObject> player = nullptr;

	{
		const auto& ar = RoomManager::singleton->getWindowAspectRatio();
		const auto& p = std::make_shared<Player>(GOL_PERSP_SECOND, shootCallback);
		p->mPosition = { fSquare, -13.0f - len, 0.0f };
		p->mCameraDist = (50.0f / (1.0f / ar) * 0.95f) + fSquare;
		p->postConstruct();
		player = RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p, true);
	}

	// Get middle Y position
	const auto yp = player->mPosition.y() * 0.5f - 1.0f;

	// Create player limit line, just above the player
	{
		std::shared_ptr<LimitLine> p = std::make_shared<LimitLine>(GOL_PERSP_SECOND, Color4{ 1.0f, 0.0f, 0.0f, 1.0f }, GO_LL_TYPE_RED);
		p->mPosition = { fSquare, player->mPosition.y() + 6.0f, 0.1f };
		p->setScale(Vector3(100.0f, 0.2f, 1.0f));
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p);
	}

	// Create left and right limit line
	for (UnsignedInt i = 0; i < 2; ++i)
	{
		const Float xp = i ? len + 50.0f : -50.0f;
		std::shared_ptr<LimitLine> p = std::make_shared<LimitLine>(GOL_PERSP_SECOND, Color4{ 0.0f, 0.0f, 0.0f, 0.2f }, GO_LL_TYPE_BLACK);
		p->mPosition = { xp, player->mPosition.y() + 56.0f, 0.0f };
		p->setScale(Vector3(50.0f, 50.0f, 1.0f));
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p);
	}

	// Setup camera for game layers
	{
		auto& gol = mGoLayers[GOL_PERSP_SECOND];
		gol.cameraEye = { fSquare, yp, 1.0f };
		gol.cameraTarget = { fSquare, yp, 0.0f };
	}

	/*
	mCameraEye = { 20.0f, -35.0f, 20.0f };
	mCameraTarget = { 8.0f, -35.0f, 0.0f };
	*/
}

std::unique_ptr<RoomManager::Instantiator> RoomManager::getGameObjectFromNoiseValue(const double value)
{
	std::unique_ptr<Instantiator> d = nullptr;

	if (value >= 0.0)
	{
		const Int index = Int(Math::round(value * double(mBubbleColors.size())));
		const auto& it = std::next(std::begin(mBubbleColors), index % mBubbleColors.size());

		if (!CommonUtility::singleton->isBubbleColorValid(it->second.color))
		{
			return d;
		}
		d = std::make_unique<Instantiator>();
		 
		nlohmann::json params;
		params["parent"] = GOL_PERSP_SECOND;
		params["color"] = {};

		if (value == 0.5)
		{
			params["color"]["r"] = mBubbleColors[BUBBLE_COIN.toSrgbInt()].color.r();
			params["color"]["g"] = mBubbleColors[BUBBLE_COIN.toSrgbInt()].color.g();
			params["color"]["b"] = mBubbleColors[BUBBLE_COIN.toSrgbInt()].color.b();
		}
		else
		{
			params["color"]["r"] = it->second.color.r();
			params["color"]["g"] = it->second.color.g();
			params["color"]["b"] = it->second.color.b();
		}
		
		d->key = GOT_BUBBLE;
		d->params = std::make_unique<nlohmann::json>(params);
	}
	return d;
}