#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "Engine.h"
#include "InputManager.h"
#include "AssetManager.h"
#include "RoomManager.h"
#include "GameObject.h"

Engine::Engine(const Arguments& arguments) :
	Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }
{
	// Setup window
	#ifdef MAGNUM_SDL2APPLICATION_MAIN
	setWindowSize({ 432, 768 });
	#endif

	// Start timeline
	mTimeline.start();

	// Enable renderer features
	GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
	GL::Renderer::setClearColor(Color4({ 0.25f, 0.25f, 0.25f, 1.0f }));

	InputManager::singleton = std::make_shared<InputManager>();

	AssetManager::singleton = std::make_shared<AssetManager>();

	RoomManager::singleton = std::make_shared<RoomManager>();
	RoomManager::singleton->setupRoom();
	RoomManager::singleton->createTestRoom();
}

void Engine::tickEvent()
{
	// Update input mouse events
	InputManager::singleton->updateMouseStates();

	// Compute delta time
	mDeltaTime = mTimeline.previousFrameDuration();

	// RoomManager::singleton->cameraEye += Vector3(0, 0, deltaTime);
	RoomManager::singleton->mCameraObject.setTransformation(Matrix4::lookAt(RoomManager::singleton->mCameraEye, RoomManager::singleton->mCameraTarget, Vector3::yAxis()));

	auto ws = windowSize();
	RoomManager::singleton->windowSize = ws;
	RoomManager::singleton->mCamera->setViewport(ws);

	// Update all game objects
	for (UnsignedInt i = 0; i < RoomManager::singleton->mGameObjects.size(); ++i)
	{
		std::shared_ptr<GameObject> go = RoomManager::singleton->mGameObjects[i];
		go->deltaTime = mDeltaTime;
		go->update();
	}

	// Trigger draw event
	redraw();

	// Advance timeline
	mTimeline.nextFrame();
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
	// Update state for pressed mouse button
	updateMouseButtonState(event, true);

	// Capture event
	event.setAccepted();
}

void Engine::mouseReleaseEvent(MouseEvent& event)
{
	// Update state for pressed mouse button
	updateMouseButtonState(event, false);

	// Capture event
	event.setAccepted();
}

void Engine::mouseMoveEvent(MouseMoveEvent& event)
{
	/*
	if (!(event.buttons() & MouseMoveEvent::Button::Left))
		return;
	*/

	// Update state for all mouse buttons
	updateMouseButtonStates(event);

	// Capture event
	event.setAccepted();
}

void Engine::viewportEvent(ViewportEvent& event)
{
	// Update viewports
	GL::defaultFramebuffer.setViewport(Range2Di({ 0, 0 }, event.windowSize()));
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

	// Clear input manager
	InputManager::singleton = nullptr;

	// Clear asset manager
	AssetManager::singleton = nullptr;

	// Pass default behaviour
	event.setAccepted();
}

void Engine::updateMouseButtonState(const MouseEvent& event, const bool & pressed)
{
	// Update state for the button which triggered the event
	InputManager::singleton->setMouseState(event.button(), pressed);
}

void Engine::updateMouseButtonStates(const MouseMoveEvent& event)
{
	// Get current mouse position
	InputManager::singleton->mMousePosition = event.position();

	// Get pressed buttons for this mouse move event
	const auto& mouseButtons = event.buttons();

	// Check if left button is actually pressed
	const auto& value = mouseButtons & MouseMoveEvent::Button::Left;
	InputManager::singleton->setMouseState(ImMouseButtons::Left, value ? true : false);
}