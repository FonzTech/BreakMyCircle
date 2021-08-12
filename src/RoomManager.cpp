#include <vector>
#include <unordered_set>
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
#include "RoomManager.h"

using namespace Magnum::Math::Literals;

std::unique_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::RoomManager()
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
}

RoomManager::~RoomManager()
{
	clear();
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
	auto content = Utility::Directory::readString("rooms/" + name + ".txt");
	auto roomData = nlohmann::json::parse(content);

	// Load audio
	{
		auto it = roomData.find("bgmusic");
		if (it != roomData.end())
		{
			std::string bgmusic = it->get<std::string>();

			mBgMusic = std::make_unique<StreamedAudioPlayable>(&mCameraObject);
			mBgMusic->loadAudio(bgmusic);
			mBgMusic->playable()->source()
				.setMinGain(0.25f)
				.setMaxGain(0.25f);
		}
	}

	// Iterate through list
	const std::vector<nlohmann::json> list = roomData["objects"].get<std::vector<nlohmann::json>>();
	for (auto& item : list)
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

void RoomManager::createLevelRoom()
{
	// Delete game level layer
	mGoLayers[GOL_PERSP_SECOND].list->clear();

	// Create bubbles
	const siv::PerlinNoise perlin(mSeed);

	const Int square = 8;
	const Float fSquare(square);

	for (Int i = 0; i < square; ++i)
	{
		for (Int j = 0; j < square; ++j)
		{
			// Working variables
			Float y = (Float) i;
			Float x = (Float) j;

			// Get noise value at this position
			const double dx(1.0f / fSquare * x);
			const double dy(1.0f / fSquare * y);
			const double value = perlin.accumulatedOctaveNoise2D_0_1(dx, dy, 8);

			// Work with noise value to get the actual in-game object
			const Instantiator d = getGameObjectFromNoiseValue(value);
			const auto &it = gameObjectCreators.find(d.key);
			if (it == gameObjectCreators.end())
			{
				printf("Could not find instantiator function for type %u. Skipping it.\n", d.key);
				continue;
			}

			const auto& fx = it->second;
			const auto& gameObject = fx(*d.params);

			// Get position for object
			Float startX;
			if (i % 2)
			{
				if (j == 7)
				{
					break;
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

	// Create player
	std::shared_ptr<GameObject> player = nullptr;

	{
		const auto& p = std::make_shared<Player>(GOL_PERSP_SECOND);
		p->mPosition = { 8.0f, -35.0f, 0.0f };
		player = RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p, true);
	}

	// Create limit line, just above the player
	{
		std::shared_ptr<LimitLine> p = std::make_shared<LimitLine>(GOL_PERSP_SECOND);
		p->mPosition = { 8.0f, player->mPosition.y() + 6.0f, 0.0f };
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p);
	}

	// Setup camera for game layers
	{
		auto& gol = mGoLayers[GOL_PERSP_SECOND];
		gol.cameraEye = { 8.0f, -19.0f, 1.0f };
		gol.cameraTarget = { 8.0f, -19.0f, 0.0f };
	}

	/*
	mCameraEye = { 20.0f, -35.0f, 20.0f };
	mCameraTarget = { 8.0f, -35.0f, 0.0f };
	*/
}

RoomManager::Instantiator RoomManager::getGameObjectFromNoiseValue(const double value)
{
	Instantiator d;

	if (value >= 0.0)
	{
		const Int index = Int(Math::floor(value * double(mBubbleColors.size() * 16))) % mBubbleColors.size();
		const auto& it = std::next(std::begin(mBubbleColors), std::rand() % mBubbleColors.size());

		nlohmann::json params;
		params["parent"] = GOL_PERSP_SECOND;
		params["color"] = {};
		params["color"]["r"] = it->second.color.r();
		params["color"]["g"] = it->second.color.g();
		params["color"]["b"] = it->second.color.b();
		
		d.key = GOT_BUBBLE;
		d.params = std::make_unique<nlohmann::json>(params);
	}
	return d;
}