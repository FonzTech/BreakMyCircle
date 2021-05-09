#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "Engine.h"
#include "GameObject.h"
#include "RoomManager.h"
#include "Bubble.h"

Engine::Engine(const Arguments& arguments) :
	Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }
{
	// Start timeline
	timeline.start();

	// Enable renderer features
	GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
	GL::Renderer::setClearColor(Magnum::Color4({ 0.25f, 0.25f, 0.25f, 1.0f }));

	// Create required singletons
	RoomManager::singleton = std::make_shared<RoomManager>();
	RoomManager::singleton->gameObjects.push_back(std::make_shared<Bubble>());
}

void Engine::tickEvent()
{
	// Compute delta time
	deltaTime = 15.0f * timeline.previousFrameDuration();

	// Set common data for GameObject
	GameObject::windowSize = Vector2{ windowSize() };

	// Update all game objects
	for (unsigned int i = 0; i < RoomManager::singleton->gameObjects.size(); ++i)
	{
		std::shared_ptr<GameObject> go = RoomManager::singleton->gameObjects[i];
		go->update();
	}

	// Trigger draw event
	redraw();
}

void Engine::drawEvent()
{
	// Clear buffer
	GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

	// Draw all game objects
	for (unsigned int i = 0; i < RoomManager::singleton->gameObjects.size(); ++i)
	{
		std::shared_ptr<GameObject> go = RoomManager::singleton->gameObjects[i];
		go->draw();
	}

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
	GameObject::windowSize = Vector2(event.windowSize());
}

void Engine::exitEvent(ExitEvent& event)
{
	if (RoomManager::singleton != nullptr)
	{
		RoomManager::singleton->clear();
		RoomManager::singleton = nullptr;
	}

	event.setAccepted();
}