#include "FallingBubble.h"

#include <thread>

#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "CommonUtility.h"
#include "ColoredDrawable.h"
#include "RoomManager.h"
#include "SpriteShader.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

FallingBubble::FallingBubble(const Color3& ambientColor, const bool spark) : GameObject()
{
	// Assign members
	mAmbientColor = ambientColor;
	mSpark = spark;

	mDelay = Float(std::rand() % 250) * 0.001f;

	mDiffuseColor = 0xffffff_rgbf;
	mVelocity = { 0.0f };

	// Create game bubble
	{
		std::shared_ptr<ColoredDrawable> cd = CommonUtility::createGameSphere(*mManipulator, mAmbientColor, this);
		drawables.emplace_back(cd);
	}

	// Create sparkle plane
	if (mSpark)
	{
		/* Load TGA importer plugin */
		PluginManager::Manager<Trade::AbstractImporter> manager;
		Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("PngImporter");

		if (!importer || !importer->openFile("textures/sparkles.png"))
		{
			std::exit(2);
		}

		// Set texture data and parameters
		Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
		CORRADE_INTERNAL_ASSERT(image);

		std::shared_ptr<GL::Texture2D> texture = std::make_shared<GL::Texture2D>();
		(*texture.get())
			.setWrapping(GL::SamplerWrapping::ClampToEdge)
			.setMagnificationFilter(GL::SamplerFilter::Linear)
			.setMinificationFilter(GL::SamplerFilter::Linear)
			.setStorage(1, GL::textureFormat(image->format()), image->size())
			.setSubImage(0, {}, *image);

		// Create plane
		std::shared_ptr<TexturedDrawable> td = createPlane(*mManipulator, texture);
		drawables.emplace_back(td);
	}
}

Int FallingBubble::getType()
{
	return GOT_FALLING_BUBBLE;
}

void FallingBubble::update()
{
	// Update motion
	if (mSpark)
	{
	}
	else
	{
		// Advance falling animation
		if (mDelay > 0.0f)
		{
			mDelay -= mDeltaTime;
		}
		else
		{
			mVelocity += { 0.0f, -2.0f, 0.0f };
			position += mVelocity * mDeltaTime;
		}

		// Check for off-screen position
		if (position.y() < -300.0f)
		{
			destroyMe = true;
			return;
		}
	}

	// Update transformations
	for (auto& d : drawables)
	{
		d->setTransformation(Matrix4::translation(position));
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == drawables.at(0).get())
	{
		(*(Shaders::Phong*) baseDrawable->mShader.get())
			.setLightPositions({ position + Vector3({ 0.0f, 40.0f, 5.0f }) })
			.setDiffuseColor(mDiffuseColor)
			.setAmbientColor(mAmbientColor)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		(*(SpriteShader*)baseDrawable->mShader.get())
			.bindTexture(*baseDrawable->mTexture)
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.draw(*baseDrawable->mMesh);
	}
}

void FallingBubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

std::shared_ptr<TexturedDrawable> FallingBubble::createPlane(Object3D & parent, std::shared_ptr<GL::Texture2D> & texture)
{
	// Create test mesh
	Trade::MeshData meshData = Primitives::planeSolid();

	GL::Buffer vertices;
	vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.normalsAsArray()));

	std::shared_ptr<GL::Mesh> mesh = std::make_shared<GL::Mesh>();
	(*mesh.get())
		.setPrimitive(meshData.primitive())
		.setCount(meshData.vertexCount())
		.addVertexBuffer(vertices, 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{});

	// Create Phong shader
	std::shared_ptr<SpriteShader> shader = std::make_shared<SpriteShader>();

	// Create colored drawable
	std::shared_ptr<TexturedDrawable> td = std::make_shared<TexturedDrawable>(RoomManager::singleton->mDrawables, shader, mesh, texture);
	td->setParent(&parent);
	td->setDrawCallback(this);
	return td;
}