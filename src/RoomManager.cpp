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

std::unordered_map<UnsignedInt, RoomManager::BubbleData> RoomManager::sBubbleColors = {
	{ BUBBLE_COLOR_RED.toSrgbInt(), { BUBBLE_COLOR_RED, RESOURCE_TEXTURE_BUBBLE_RED } },
	{ BUBBLE_COLOR_GREEN.toSrgbInt(), { BUBBLE_COLOR_GREEN, RESOURCE_TEXTURE_BUBBLE_GREEN } },
	{ BUBBLE_COLOR_BLUE.toSrgbInt(), { BUBBLE_COLOR_BLUE, RESOURCE_TEXTURE_BUBBLE_BLUE } },
	{ BUBBLE_COLOR_YELLOW.toSrgbInt(), { BUBBLE_COLOR_YELLOW, RESOURCE_TEXTURE_BUBBLE_YELLOW } },
	{ BUBBLE_COLOR_PURPLE.toSrgbInt(), { BUBBLE_COLOR_PURPLE, RESOURCE_TEXTURE_BUBBLE_PURPLE } },
	{ BUBBLE_COLOR_ORANGE.toSrgbInt(), { BUBBLE_COLOR_ORANGE, RESOURCE_TEXTURE_BUBBLE_ORANGE } },
	{ BUBBLE_COLOR_CYAN.toSrgbInt(), { BUBBLE_COLOR_CYAN, RESOURCE_TEXTURE_BUBBLE_CYAN } },
	{ BUBBLE_COIN.toSrgbInt(), { BUBBLE_COIN, RESOURCE_TEXTURE_BUBBLE_TRANSLUCENT } },
	{ BUBBLE_BOMB.toSrgbInt(), { BUBBLE_BOMB, RESOURCE_TEXTURE_WHITE } },
	{ BUBBLE_PLASMA.toSrgbInt(), { BUBBLE_PLASMA, RESOURCE_TEXTURE_WHITE } },
	{ BUBBLE_ELECTRIC.toSrgbInt(), { BUBBLE_ELECTRIC, RESOURCE_TEXTURE_WHITE } },
	{ BUBBLE_STONE.toSrgbInt(), { BUBBLE_STONE, RESOURCE_TEXTURE_WHITE } },
	{ BUBBLE_BLACKHOLE.toSrgbInt(), { BUBBLE_BLACKHOLE, RESOURCE_TEXTURE_WHITE } },
	{ BUBBLE_TIMED.toSrgbInt(), { BUBBLE_TIMED, RESOURCE_TEXTURE_WHITE } }
};

std::array<UnsignedInt, 7U> RoomManager::sBubbleKeys = {
	BUBBLE_COLOR_RED.toSrgbInt(),
	BUBBLE_COLOR_GREEN.toSrgbInt(),
	BUBBLE_COLOR_BLUE.toSrgbInt(),
	BUBBLE_COLOR_YELLOW.toSrgbInt(),
	BUBBLE_COLOR_PURPLE.toSrgbInt(),
	BUBBLE_COLOR_ORANGE.toSrgbInt(),
	BUBBLE_COLOR_CYAN.toSrgbInt()
};

std::unique_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::SaveData::SaveData()
{
}

RoomManager::SaveData::~SaveData()
{
	Debug{} << "Save data was destroyed";
}

bool RoomManager::SaveData::load()
{
	// Set default value
	{
#if DEBUG
		maxLevelId = 40U;
		coinTotal = 50;
		coinCurrent = 0;
#else
		maxLevelId = 2U;
		coinTotal = 0;
		coinCurrent = 0;
#endif

		flags = 0U;
		onboardIndex = 0; 
		sfxEnabled = true;
		musicEnabled = true;

		for (UnsignedInt i = 0; i < GO_LS_MAX_POWERUP_COUNT; ++i)
		{
#if DEBUG
			powerupAmounts[GO_LS_GUI_POWERUP + i] = 2;
#else
			powerupAmounts[GO_LS_GUI_POWERUP + i] = 0;
#endif
		}
	}

	// Check if file path is valid
	if (CommonUtility::singleton->mConfig.saveFile.empty())
	{
		Error{} << "Save file to read from was NULL";
		return false;
	}

	// Load JSON data
	nlohmann::json jsonData;
	{
		const auto rawJson = Utility::Directory::readString(CommonUtility::singleton->mConfig.saveFile);
		try
		{
			jsonData = nlohmann::json::parse(rawJson);
		}
		catch (const nlohmann::json::parse_error& ex)
		{
			Error{} << "Parse error at byte " << ex.byte;
			return false;
		}
	}

	// Flags
	{
		const auto value = jsonData.find("flags");
		flags = value != jsonData.end() ? (*value).get<UnsignedInt>() : 0U;
	}

	// Max level ID
	{
		const auto value = jsonData.find("maxLevelId");
		maxLevelId = value != jsonData.end() ? (*value).get<UnsignedInt>() : 2U;
	}

	// Coin total
	{
		const auto value = jsonData.find("coinTotal");
		coinTotal = value != jsonData.end() ? (*value).get<Int>() : 0;
	}

	// Coin current
	{
		const auto value = jsonData.find("coinCurrent");
		coinCurrent = value != jsonData.end() ? (*value).get<Int>() : 0;
	}

	// Load powerup amounts
	const std::array<std::string, 2> entries = { "powerupAmounts", "levelScores" };
	for (auto i = 0; i < entries.size(); ++i)
	{
		const auto jsonMap = jsonData.find(entries.at(i));
		if (jsonMap != jsonData.end())
		{
			for (auto& item : (*jsonMap).items())
			{
				const auto key = UnsignedInt(std::stoi(item.key()));
				(i == 0 ? powerupAmounts : levelScores)[key] = item.value().get<Int>();
			}
		}
	}

	// Onboard index
	{
		const auto value = jsonData.find("onboardIndex");
		onboardIndex = value != jsonData.end() ? (*value).get<Int>() : 0;
	}

	// Music enabled
	{
		const auto value = jsonData.find("musicEnabled");
		musicEnabled = value != jsonData.end() ? (*value).get<Int>() == 1 : true;
	}

	// SFX enabled
	{
		const auto value = jsonData.find("sfxEnabled");
		sfxEnabled = value != jsonData.end() ? (*value).get<Int>() == 1 : true;
	}

	return true;
}

bool RoomManager::SaveData::save()
{
	// Check if file path is valid
	if (CommonUtility::singleton->mConfig.saveFile.empty())
	{
		Error{} << "Save file to save to was NULL";
		return false;
	}

	// Dump JSON data to file
	nlohmann::json jsonData = {
		{ "flags", flags },
		{ "maxLevelId", maxLevelId },
		{ "coinTotal", coinTotal },
		{ "coinCurrent", coinCurrent },
		{ "powerupAmounts", std::unordered_map<std::string, Int>() },
		{ "levelScores", std::unordered_map<std::string, Int>() },
		{ "onboardIndex", onboardIndex },
		{ "musicEnabled", musicEnabled ? 1 : 0 },
		{ "sfxEnabled", sfxEnabled ? 1 : 0 },
	};

	for (const auto& item : powerupAmounts)
	{
		const auto& key = std::to_string(item.first);
		jsonData["powerupAmounts"][key] = item.second;
	}

	for (const auto& item : levelScores)
	{
		const auto& key = std::to_string(item.first);
		jsonData["levelScores"][key] = item.second;
	}

	Utility::Directory::writeString(CommonUtility::singleton->mConfig.saveFile, jsonData.dump());
	return true;
}

RoomManager::RoomManager() : mCurrentBoundParentIndex(-1), mSfxLevel(1.0f)
{
	// Create audio manager
	mAudioContext = std::make_unique<Audio::Context>(
		Audio::Context::Configuration{}
		.setHrtf(Audio::Context::Configuration::Hrtf::Disabled)
		.setFrequency(44100)
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

	// Load saved gameplay
	mSaveData = SaveData();
    mSaveData.load();
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
	if (mBgMusic != nullptr)
	{
		const auto& p = mBgMusic->playable();
		if (p != nullptr)
		{
			return p->gain();
		}
		return -2.0f;
	}
	return -1.0f;
}

const bool RoomManager::setBgMusicGain(const Float level)
{
	if (mBgMusic != nullptr)
	{
		const auto& p = mBgMusic->playable();
		if (p != nullptr)
		{
			mBgMusic->playable()->setGain(level);
			return true;
		}
	}
	return false;
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
	const auto& content = Utility::Directory::readString(CommonUtility::singleton->mConfig.assetDir + "rooms/" + name + ".txt");
	const auto& roomData = nlohmann::json::parse(content);

	// Load audio
	{
		const auto& it = roomData.find("bgmusic");
		if (it != roomData.end())
		{
			const auto& bgmusic = it->get<std::string>();

			mBgMusic = std::make_unique<StreamedAudioPlayable>(&mCameraObject);
			mBgMusic->loadAudio(bgmusic);
			setBgMusicGain(mSaveData.musicEnabled ? 0.25f : 0.0f);
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
				Error{} << "Could not find instantiator function for type" << type << ". Skipping it";
				continue;
			}

			const auto& fx = it->second;
			gameObject = fx(item);
		}

		// Push into room
		mGoLayers[parent].push_back(gameObject);
	}
}

void RoomManager::createLevelRoom(const std::shared_ptr<IShootCallback> & shootCallback, const Int xlen, const Int ylen, const std::uint32_t seed, const std::int32_t octaves, const double frequency)
{
	// Delete game level layer
	mGoLayers[GOL_PERSP_SECOND].list->clear();

	// Create variables
	const double fSeed(seed);
	const Float fSquare(xlen);

	const Float len = fSquare * 2.0f; // "2" is the fixed diameter of a "game bubble"
	const Float playerY = -13.0f - len;

	// Get middle Y position
	const auto yp = playerY * 0.5f - 1.0f;

	// Create player limit line, just above the player
	{
		std::shared_ptr<LimitLine> p = std::make_shared<LimitLine>(GOL_PERSP_SECOND, Color4{ 1.0f, 0.0f, 0.0f, 1.0f }, GO_LL_TYPE_RED);
		p->mPosition = { fSquare, playerY + 6.0f, 0.2f };
		p->setScale(Vector3(100.0f, 0.2f, 1.0f));
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p);
	}

	// Create left and right limit line
	for (UnsignedInt i = 0; i < 2; ++i)
	{
		const Float xp = i ? len + 50.0f : -50.0f;
		std::shared_ptr<LimitLine> p = std::make_shared<LimitLine>(GOL_PERSP_SECOND, Color4{ 0.0f, 0.0f, 0.0f, 0.2f }, GO_LL_TYPE_BLACK);
		p->mPosition = { xp, playerY + 56.0f, 0.1f };
		p->setScale(Vector3(50.0f, 50.0f, 1.0f));
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p);
	}

	// Top limit line
	{
		std::shared_ptr<LimitLine> p = std::make_shared<LimitLine>(GOL_PERSP_SECOND, Color4{ 0.25f, 0.25f, 0.25f, 1.0f }, GO_LL_TYPE_BLACK);
		p->mPosition = { 0.0f, 1.25f, 0.1f };
		p->setScale(Vector3(16.0f, 0.2f, 1.0f));
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(p);
	}

	// Create bubbles
	const siv::PerlinNoise perlin(seed);

	const double xf = xlen / frequency;
	const double yf = ylen / frequency;

	Float pzBh = 0.0f;

	for (Int i = 0; i < ylen; ++i)
	{
		for (Int j = 0; j < xlen; ++j)
		{
			// Working variables
			double y = (double) i;
			double x = (double) j;
			Float pz = 0.0f;

			// Work with noise value to get the actual in-game object
			std::unique_ptr<Instantiator> pi = nullptr;
			{
				// Get valid instantiator
				double ox = fSeed * x * 0.01;
				while (true)
				{
					const double value = perlin.accumulatedOctaveNoise2D_0_1((x + ox) / xf, y + y / yf, octaves);
					pi = getGameObjectFromNoiseValue(seed, value);
					if (pi != nullptr)
					{
						// Check if object is a bubble
						const auto& c1 = pi->params->find("color");
						if (c1 == pi->params->end())
						{
							break;
						}

						// Don't place any bubble coin at the top (because they can't be exploded without powerups)
						const auto& c2 = c1->find("int");
						if (*c2 != BUBBLE_COIN.toSrgbInt() || i > 1)
						{
							if (*c2 == BUBBLE_BLACKHOLE.toSrgbInt())
							{
								pzBh += 0.05f;
								pz = pzBh;
							}
							break;
						}
					}

					ox += 0.1;
				}
			}

			if (pi->key == -1)
			{
				continue;
			}

			const auto &it = gameObjectCreators.find(pi->key);
			if (it == gameObjectCreators.end())
			{
				Debug{} << "Could not find instantiator function for type " << pi->key << ". Skipping it";
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

			gameObject->mPosition = Vector3{ startX + Float(x) * 2.0f, Float(y) * -2.0f, pz };
			RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(gameObject);
		}
	}

	// Fix transparency issues for some bubbles
	fixLevelTransparency();

	// Setup projectile parameters
	Projectile::setGlobalParameters(1.0f, len - 1.0f);

	// Create player
	std::shared_ptr<Player> player = nullptr;
	{
		player = std::make_shared<Player>(GOL_PERSP_SECOND, shootCallback);
		player->mPosition = { fSquare, playerY, 0.0f };
		player->mCameraDist = 1.0f;
		player->mCameraDistBase = fSquare;
		RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].push_back(player);
	}

	// Call post-construct for player
	player->postConstruct();

	// Setup camera for game layers
	{
		auto& gol = mGoLayers[GOL_PERSP_SECOND];
		gol.cameraEye = { fSquare, yp, 1.0f };
		gol.cameraTarget = { fSquare, yp, 0.0f };
	}
}

std::unique_ptr<RoomManager::Instantiator> RoomManager::getGameObjectFromNoiseValue(const std::uint32_t seed, const double value)
{
	std::unique_ptr<Instantiator> d = nullptr;

	if (value >= 0.0)
	{
		const double maxIndex = 4 + Int(Float(Int(seed - 1) % 100) * 0.1f);
		const Int index = (Int(seed) + Int(Math::round(value * maxIndex))) % Int(sBubbleKeys.size());
		const auto& bd = sBubbleKeys[index];

		d = std::make_unique<Instantiator>();

		nlohmann::json params;
		params["parent"] = GOL_PERSP_SECOND;

		const bool isHole = isInRange(value, 0.09) || isInRange(value, 0.54) || isInRange(value, 0.98);
		const bool isCoin = isInRange(value, 0.15, 0.03) || isInRange(value, 0.65, 0.03) || isInRange(value, 0.90, 0.03);

		const double stf = double(seed % 25U) / 25.0 * 0.01;
		const bool isStone = isInRange(value, 0.02, stf) || isInRange(value, 0.35, stf) || isInRange(value, 0.89, stf);

		const double bhf = double(seed % 50U) / 50.0 * 0.01;
		const bool isBlackhole = isInRange(value, 0.10, bhf) || isInRange(value, 0.42, bhf) || isInRange(value, 0.76, bhf);

		const double tif = double(seed % 30U) / 10.0 * 0.01;
		const bool isTimed = isInRange(value, 0.06, tif) || isInRange(value, 0.3, tif) || isInRange(value, 0.82, tif);

		if (isHole)
		{
			d->key = -1;
		}
		else if (isCoin || isStone || isBlackhole || isTimed)
		{
			UnsignedInt k = 0U;
			if (isCoin)
			{
				k = BUBBLE_COIN.toSrgbInt();
			}
			else if (isStone)
			{
				k = BUBBLE_STONE.toSrgbInt();
			}
			else if (isBlackhole)
			{
				k = BUBBLE_BLACKHOLE.toSrgbInt();
			}
			else if (isTimed)
			{
				k = BUBBLE_TIMED.toSrgbInt();
				params["timedDelay"] = std::fmodf(Float(seed % 10U) * 0.1f + Float(value) * 10.0f, 1.0f);
			}

			const auto& vd = sBubbleColors[k];
			params["color"] = {
				{ "int", k },
				{ "r", vd.color.r() },
				{ "g", vd.color.g() },
				{ "b", vd.color.b() }
			};

			d->key = GOT_BUBBLE;
		}
		else
		{
			const auto& vd = sBubbleColors[bd];
			params["color"] = {
				{ "int", vd.color.toSrgbInt() },
				{ "r", vd.color.r() },
				{ "g", vd.color.g() },
				{ "b", vd.color.b() }
			};

			d->key = GOT_BUBBLE;
		}
		
		d->params = std::make_unique<nlohmann::json>(params);
	}
	return d;
}

Float RoomManager::isInRange(const double source, const double dest, const double range)
{
	return Math::abs(source - dest) < range;
}

void RoomManager::fixLevelTransparency()
{
	for (auto& item : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
	{
		if (item->getType() == GOT_BUBBLE)
		{
			auto& p = (std::shared_ptr<Bubble>&)item;
			if (p->mAmbientColor == BUBBLE_BLACKHOLE)
			{
				p->pushToFront();
			}
		}
	}
}
