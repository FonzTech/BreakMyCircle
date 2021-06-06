#include <vector>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "RoomManager.h"
#include "Bubble.h"
#include "Player.h"
#include "Scenery.h"

using namespace Magnum::Math::Literals;

std::unique_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::RoomManager()
{
	// Initialize camera members
	mCameraEye = { 0.0f, 0.0f, 20.0f };
	mCameraTarget = { 0.0f, 0.0f, 0.0f };

	// Create collision manager
	mCollisionManager = std::make_unique<CollisionManager>();
}

void RoomManager::clear()
{
	mGameObjects.clear();

	mCamera = nullptr;
}

void RoomManager::setupRoom()
{
	mCameraObject.setParent(&mScene);

	mCamera = std::make_shared<SceneGraph::Camera3D>(mCameraObject);
	mCamera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);
	mCamera->setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 10000.0f));
	mCamera->setViewport(GL::defaultFramebuffer.viewport().size());
}

void RoomManager::createTestRoom()
{
	// Available colors
	std::vector<Color3> colors = {
		0x0000c0_rgbf,
		0x00c000_rgbf,
		0xc00000_rgbf,
		0x00c0c0_rgbf,
	};

	// Create bubbles
	const Int square = 8;
	for (Int i = 0; i < square; ++i)
	{
		for (Int j = 0; j < square; ++j)
		{
			Int index = std::rand() % colors.size();

			if (i == 6 && j < 4 || i == 7 && j == 3)
			{
				index = 0;
			}

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

			Float y = (Float) i;
			Float x = (Float) j;

			Vector3 position = { startX + x * 2.0f, y * -2.0f, 0.0f };

			std::shared_ptr<Bubble> b = std::make_shared<Bubble>(colors[index]);
			b->position = position;
			b->updateBBox();
			RoomManager::singleton->mGameObjects.push_back(std::move(b));
		}
	}

	// Create player
	{
		std::shared_ptr<Player> p = std::make_shared<Player>();
		p->position = { 8.0f, -35.0f, 0.0f };
		RoomManager::singleton->mGameObjects.push_back(std::move(p));
	}

	// Create scenery
	{
		std::shared_ptr<Scenery> p = std::make_shared<Scenery>();
		p->position = Vector3(0.0f);
		RoomManager::singleton->mGameObjects.push_back(std::move(p)); 
	}

	// Camera position
	mCameraEye = { 8.0f, -20.0f, 44.0f };
	mCameraTarget = { 8.0f, -20.0f, 0.0f };
}