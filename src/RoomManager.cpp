#include <vector>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "RoomManager.h"
#include "Bubble.h"
#include "Player.h"


using namespace Magnum::Math::Literals;

std::shared_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::RoomManager()
{
	cameraEye = { 0.0f, 0.0f, 20.0f };
	cameraTarget = { 0.0f, 0.0f, 0.0f };
}

void RoomManager::clear()
{
	mGameObjects.clear();
}

void RoomManager::setupRoom()
{
	mCameraObject.setParent(&mScene);

	mCamera = std::make_shared<SceneGraph::Camera3D>(mCameraObject);
	mCamera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);
	mCamera->setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 1000.0f));
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
	for (UnsignedInt i = 0; i < 10; ++i)
	{
		for (UnsignedInt j = 0; j < 10; ++j)
		{
			UnsignedInt index = std::rand() % colors.size();

			Float startX;
			if (i % 2)
			{
				if (j == 9)
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

			std::shared_ptr<Bubble> b = std::make_shared<Bubble>(RoomManager::singleton->mDrawables);
			b->setParent(&RoomManager::singleton->mScene);
			b->position = { startX + x * 2.0f, y * -2.0f, 0.0f };
			b->mAmbientColor = colors[index];
			RoomManager::singleton->mGameObjects.push_back(b);
		}
	}

	// Create player
	std::shared_ptr<Player> p = std::make_shared<Player>(RoomManager::singleton->mDrawables);
	p->setParent(&RoomManager::singleton->mScene);
	p->position = { 10.0f, -40.0f, 0.0f };
	RoomManager::singleton->mGameObjects.push_back(p);

	// Camera position
	cameraEye = { 10.0f, -20.0f, 50.0f };
	cameraTarget = { 10.0f, -20.0f, 0.0f };
}