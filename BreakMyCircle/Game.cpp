#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/DefaultFramebuffer.h>

#include "Game.h"
#include "GameObject.h"
#include "RoomManager.h"
#include "Bubble.h"

Game::Game(const Arguments& arguments) :
	Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }
{
	// Start timeline
	timeline.start();

	// Create required singletons
	RoomManager::singleton = std::make_shared<RoomManager>();
	RoomManager::singleton->gameObjects.push_back(std::make_shared<Bubble>());

	// Enable renderer features
	GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
	GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
}

void Game::tickEvent()
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

void Game::drawEvent()
{
	// Clear buffer
	GL::defaultFramebuffer.clearColor(Magnum::Color4({ 1.0f, 0.0f, 0.0f, 1.0f }));
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

void Game::mousePressEvent(MouseEvent& event)
{
	if (event.button() != MouseEvent::Button::Left)
		return;

	event.setAccepted();
}

void Game::mouseReleaseEvent(MouseEvent& event)
{
	event.setAccepted();
}

void Game::mouseMoveEvent(MouseMoveEvent& event)
{
	if (!(event.buttons() & MouseMoveEvent::Button::Left))
		return;

	event.setAccepted();
}

void Game::exitEvent(ExitEvent& event)
{
	if (RoomManager::singleton != nullptr)
	{
		RoomManager::singleton->clear();
		RoomManager::singleton = nullptr;
	}

	event.setAccepted();
}