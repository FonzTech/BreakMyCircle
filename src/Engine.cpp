#include <memory>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/TextureFormat.h>

#include "Engine.h"
#include "CommonUtility.h"
#include "InputManager.h"
#include "RoomManager.h"
#include "GameObject.h"

const Sint8 Engine::GO_LAYERS[] = {
	GOL_MAIN,
	GOL_LEVEL
};

Engine::Engine(const Arguments& arguments) : Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }
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
	GL::Renderer::enable(GL::Renderer::Feature::Blending);
	GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
	GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);

	// Set clear color
	GL::Renderer::setClearColor(Color4(0.2f, 0.2f, 0.2f, 1.0f));

	// Init common utility
	CommonUtility::singleton = std::make_unique<CommonUtility>();

	// Init input manager
	InputManager::singleton = std::make_unique<InputManager>();

	// Init room manager
	RoomManager::singleton = std::make_unique<RoomManager>();
	RoomManager::singleton->setup();

	// Setup room manager
	upsertGameObjectLayers();

	// Build room
	RoomManager::singleton->loadRoom("intro");
	// RoomManager::singleton->createLevelRoom();
}

void Engine::tickEvent()
{
	// Update input events
	InputManager::singleton->updateMouseStates();
	InputManager::singleton->updateKeyStates();

	// Compute delta time
	mDeltaTime = mTimeline.previousFrameDuration();

	auto ws = windowSize();
	RoomManager::singleton->windowSize = ws;
	RoomManager::singleton->mCamera->setViewport(ws);

	// Iterate through all layers
	for (auto& gol : RoomManager::singleton->mGoLayers)
	{
		// Bind layer's framebuffer
		currentGol = &gol.second;
		(*currentGol->frameBuffer)
			.clear(GL::FramebufferClear::Depth)
			.clearColor(GLF_COLOR_ATTACHMENT_INDEX, Color4(0.0f, 0.0f, 0.0f, 0.0f))
			.bind();

		// Position camera on this layer
		RoomManager::singleton->mCameraObject.setTransformation(Matrix4::lookAt(currentGol->mCameraEye, currentGol->mCameraTarget, Vector3::yAxis()));

		// Get vector as reference
		const auto& gos = gol.second.list;

		// Update all game objects on this layer
		for (UnsignedInt i = 0; i < gos->size(); ++i)
		{
			std::shared_ptr<GameObject> go = gos->at(i);
			go->mDeltaTime = mDeltaTime;
			go->update();
		}

		// Destroy all marked objects as such on this layer
		for (UnsignedInt i = 0; i < gos->size();)
		{
			std::shared_ptr<GameObject> go = gos->at(i);
			if (go->destroyMe)
			{
				gos->erase(gos->begin() + i);
			}
			else
			{
				++i;
			}
		}

		// Draw all game objects on this layer
		drawEvent();

		// De-reference game object layer
		currentGol = nullptr;
	}

	// Redraw main frame buffer
	{
		// Bind default window framebuffer
		GL::defaultFramebuffer
			.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth)
			.bind();

		// Redraw
		drawEvent();
	}

	// Advance timeline
	mTimeline.nextFrame();
}

void Engine::drawEvent()
{
	// Process main frame buffer
	if (currentGol == nullptr)
	{
		// Draw screen quad
		mScreenQuadShader
			.bindTexture(GOL_MAIN, *RoomManager::singleton->mGoLayers[GOL_MAIN].fbTexture)
			.bindTexture(GOL_LEVEL, *RoomManager::singleton->mGoLayers[GOL_LEVEL].fbTexture)
			.draw(mScreenQuadShader.mMesh);

		// Swap buffers
		swapBuffers();
	}
	else
	{
		// Z ordering
		std::vector<std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>> drawableTransformations = RoomManager::singleton->mCamera->drawableTransformations(*currentGol->drawables);
		std::sort(drawableTransformations.begin(), drawableTransformations.end(),
			[](const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& a,
				const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& b) {
			return a.second.translation().z() < b.second.translation().z();
		});

		// Draw scene
		RoomManager::singleton->mCamera->draw(drawableTransformations);
	}
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
	// Update state for released mouse button
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

void Engine::keyPressEvent(KeyEvent& event)
{
	// Update state for pressed key button
	updateKeyButtonState(event, true);

	// Capture event
	event.setAccepted();
}

void Engine::keyReleaseEvent(KeyEvent& event)
{
	// Update state for released key button
	updateKeyButtonState(event, false);

	// Capture event
	event.setAccepted();
}

void Engine::viewportEvent(ViewportEvent& event)
{
	// Update viewports
	GL::defaultFramebuffer.setViewport(Range2Di({ 0, 0 }, event.windowSize()));
	RoomManager::singleton->mCamera->setViewport(event.windowSize());
	upsertGameObjectLayers();
}

void Engine::exitEvent(ExitEvent& event)
{
	/*
		Clear the entire room. Must be done now, because this
		object holds data about game objects, and so they holds
		references about meshes, textures, shaders, etc...
	*/
	if (RoomManager::singleton != nullptr)
	{
		RoomManager::singleton->clear();
	}
	RoomManager::singleton = nullptr;

	/*
		Now, common utility can be cleared, expecially because
		it contains the resource manager. It can be cleared now,
		and only now, because no more references are present
		for contained resources.
	*/
	if (CommonUtility::singleton != nullptr)
	{
		CommonUtility::singleton->clear();
	}
	CommonUtility::singleton = nullptr;

	// Clear input manager
	InputManager::singleton = nullptr;

	// Pass default behaviour
	event.setAccepted();
}

void Engine::upsertGameObjectLayers()
{
	// Cycle through all required layers
	for (const auto& index : GO_LAYERS)
	{
		// Check if layer has been already created
		RoomManager::GameObjectsLayer* layer;
		{
			auto it = RoomManager::singleton->mGoLayers.find(index);
			if (it != RoomManager::singleton->mGoLayers.end())
			{
				// Layer exists
				layer = &it->second;

				// Clear framebuffer
				layer->frameBuffer = nullptr;
			}
			else
			{
				// Create layer
				RoomManager::singleton->mGoLayers[index] = RoomManager::GameObjectsLayer();
				layer = &RoomManager::singleton->mGoLayers[index];
				layer->index = index;

				// Create game object list
				layer->list = std::make_unique<GameObjectList>();

				// Create drawables holder
				layer->drawables = std::make_unique<SceneGraph::DrawableGroup3D>();
			}
		}

		// Get size for window framebuffer
		const auto& size = GL::defaultFramebuffer.viewport().size();

		// Create main texture to attach layer
		layer->fbTexture = std::make_unique<GL::Texture2D>();

		GL::Renderbuffer depthStencil;
		layer->fbTexture->setStorage(1, GL::TextureFormat::RGBA8, size);
		depthStencil.setStorage(GL::RenderbufferFormat::Depth24Stencil8, size);

		// Create framebuffer and attach color and depth buffers
		layer->frameBuffer = std::make_unique<GL::Framebuffer>(Range2Di({}, size));
		layer->frameBuffer->attachTexture(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, *layer->fbTexture, 0);
		layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::BufferAttachment::DepthStencil, depthStencil);
	}
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

void Engine::updateKeyButtonState(const KeyEvent& event, const bool & pressed)
{
	// Update state for the button which triggered the event
	InputManager::singleton->setKeyState(event.key(), pressed);
}