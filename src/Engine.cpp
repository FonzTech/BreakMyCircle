#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "Engine.h"
#include "GameObject.h"
#include "RoomManager.h"

Engine::Engine(const Arguments& arguments) :
	Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }
{
	// Start timeline
	timeline.start();

	// Enable renderer features
	GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
	GL::Renderer::setClearColor(Color4({ 0.25f, 0.25f, 0.25f, 1.0f }));

	RoomManager::singleton = std::make_shared<RoomManager>();
	RoomManager::singleton->setupRoom();
	RoomManager::singleton->createExampleRoom();
}

void Engine::tickEvent()
{
	// Compute delta time
	deltaTime = timeline.previousFrameDuration();

	RoomManager::singleton->cameraEye += Vector3(0, 0, deltaTime);
	RoomManager::singleton->mCameraObject.setTransformation(Matrix4::lookAt(RoomManager::singleton->cameraEye, RoomManager::singleton->cameraTarget, Vector3::yAxis()));

	// Update all game objects
	for (UnsignedInt i = 0; i < RoomManager::singleton->mGameObjects.size(); ++i)
	{
		std::shared_ptr<GameObject> go = RoomManager::singleton->mGameObjects[i];
		go->deltaTime = deltaTime;
		go->update();
	}

	// Trigger draw event
	redraw();

	// Advance timeline
	timeline.nextFrame();
}

void Engine::drawEvent()
{
	// Clear buffer
	GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

	// Draw scene
	RoomManager::singleton->mCamera->draw(RoomManager::singleton->mDrawables);

	// Swap buffers
	swapBuffers();
}

void Engine::mousePressEvent(MouseEvent& event)
{
	if (event.button() != MouseEvent::Button::Left)
		return;

	event.setAccepted();
}

void Engine::mouseReleaseEvent(MouseEvent& event)
{
	event.setAccepted();
}

void Engine::mouseMoveEvent(MouseMoveEvent& event)
{
	if (!(event.buttons() & MouseMoveEvent::Button::Left))
		return;

	event.setAccepted();
}

void Engine::viewportEvent(ViewportEvent& event)
{
	// Update viewport for camera
	RoomManager::singleton->mCamera->setViewport(event.windowSize());
}

void Engine::exitEvent(ExitEvent& event)
{
	// Clear entire room
	if (RoomManager::singleton != nullptr)
	{
		RoomManager::singleton->clear();
		RoomManager::singleton = nullptr;
	}

	// Pass default behaviour
	event.setAccepted();
}