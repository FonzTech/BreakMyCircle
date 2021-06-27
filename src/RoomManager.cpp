#include <vector>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "CommonUtility.h"
#include "PerlinNoise.hpp"
#include "RoomManager.h"
#include "Player.h"
#include "Bubble.h"
#include "Projectile.h"
#include "FallingBubble.h"
#include "Scenery.h"
#include "Logo.h"

using namespace Magnum::Math::Literals;

std::unique_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::RoomManager()
{
	// Map every game object to this map
	gameObjectCreators[GOT_PLAYER] = Player::getInstance;
	gameObjectCreators[GOT_BUBBLE] = Bubble::getInstance;
	gameObjectCreators[GOT_PROJECTILE] = Projectile::getInstance;
	gameObjectCreators[GOT_FALLING_BUBBLE] = FallingBubble::getInstance;
	gameObjectCreators[GOT_SCENERY] = Scenery::getInstance;
	gameObjectCreators[GOT_LOGO] = Logo::getInstance;

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

void RoomManager::clear()
{
	// Clear all layers and their children
	mGoLayers.clear();

	// De-reference camera
	mCamera = nullptr;
}

void RoomManager::setup()
{
	// Set parent for camera
	mCameraObject.setParent(&mScene);

	// Setup camera
	mCamera = std::make_shared<SceneGraph::Camera3D>(mCameraObject);
	mCamera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);
	mCamera->setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 1000.0f));
	mCamera->setViewport(GL::defaultFramebuffer.viewport().size());
}

void RoomManager::prepareRoom()
{
	// Delete all game objects across all layers
	for (const auto& layer : mGoLayers)
	{
		layer.second.list->clear();
	}
}

void RoomManager::loadRoom(const std::string & name)
{
	// Load room from file
	auto content = Utility::Directory::readString("rooms/" + name + ".txt");
	auto list = nlohmann::json::parse(content);

	// Iterate through list
	for (auto& item : list)
	{
		// Get type
		Sint8 parent, type;
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

		// Read parameters
		{
			const auto& it = item.find("position");
			if (it != item.end())
			{
				Float position[3];
				(*it).at("x").get_to(position[0]);
				(*it).at("y").get_to(position[1]);
				(*it).at("z").get_to(position[2]);
				gameObject->position = Vector3(position[0], position[1], position[2]);
			}
		}

		// Push into room
		mGoLayers[parent].push_back(gameObject);
	}
}

void RoomManager::createLevelRoom()
{
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

			gameObject->position = position;
			RoomManager::singleton->mGoLayers[GOL_SECOND].push_back(gameObject);
		}
	}

	// Create player
	{
		std::shared_ptr<Player> p = std::make_shared<Player>(GOL_SECOND);
		p->position = { 8.0f, -35.0f, 0.0f };
		RoomManager::singleton->mGoLayers[GOL_SECOND].push_back(p);
	}

	// Create scenery
	{
		std::shared_ptr<Scenery> p = std::make_shared<Scenery>(GOL_FIRST);
		p->position = Vector3(0.0f);
		RoomManager::singleton->mGoLayers[GOL_FIRST].push_back(p);
	}

	// Camera position
	mGoLayers[GOL_FIRST].mCameraEye = { 8.0f, -20.0f, 44.0f };
	mGoLayers[GOL_FIRST].mCameraTarget = { 8.0f, -20.0f, 0.0f };

	mGoLayers[GOL_SECOND].mCameraEye = { 8.0f, -20.0f, 1.0f };
	mGoLayers[GOL_SECOND].mCameraTarget = { 8.0f, -20.0f, 0.0f };

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
		params["parent"] = GOL_SECOND;
		params["color"] = {};
		params["color"]["r"] = it->second.color.r();
		params["color"]["g"] = it->second.color.g();
		params["color"]["b"] = it->second.color.b();
		
		d.key = GOT_BUBBLE;
		d.params = std::make_unique<nlohmann::json>(params);
	}
	return d;
}