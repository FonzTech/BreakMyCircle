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

std::shared_ptr<GameObject> FallingBubble::getInstance(nlohmann::json params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

FallingBubble::FallingBubble(const Color3& ambientColor, const bool spark) : GameObject()
{
	// Assign members
	mAmbientColor = ambientColor;
	mSpark = spark;

	mDelay = Float(std::rand() % 250) * 0.001f;

	mDiffuseColor = 0xffffff_rgbf;
	mVelocity = { 0.0f };

	// Create sparkle plane
	if (mSpark)
	{
		// Get sparkles texture
		Resource<GL::Texture2D> resTexture{ CommonUtility::singleton->manager.get<GL::Texture2D>(RESOURCE_TEXTURE_SPARKLES) };

		if (!resTexture)
		{
			// Load TGA importer plugin
			PluginManager::Manager<Trade::AbstractImporter> manager;
			Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("PngImporter");

			if (!importer || !importer->openFile("textures/sparkles.png"))
			{
				std::exit(2);
			}

			// Set texture data and parameters
			Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
			CORRADE_INTERNAL_ASSERT(image);

			GL::Texture2D texture;
			texture
				.setWrapping(GL::SamplerWrapping::ClampToEdge)
				.setMagnificationFilter(GL::SamplerFilter::Linear)
				.setMinificationFilter(GL::SamplerFilter::Linear)
				.setStorage(1, GL::textureFormat(image->format()), image->size())
				.setSubImage(0, {}, *image);

			// Add to resources
			CommonUtility::singleton->manager.set(resTexture.key(), std::move(texture));
		}

		// Create plane
		std::shared_ptr<TexturedDrawable<SpriteShader>> td = createPlane(*mManipulator, resTexture);
		mDrawables.emplace_back(td);

		// Create shader data wrapper
		wrapper.shader = &td->getShader();
		wrapper.parameters.index = 0.0f;
		wrapper.parameters.total = 16.0f;
		wrapper.parameters.texWidth = Float(td->mTexture->imageSize(0).x());
		wrapper.parameters.texHeight = Float(td->mTexture->imageSize(0).y());
		wrapper.parameters.rows = 4.0f;
		wrapper.parameters.columns = 4.0f;
		wrapper.speed = 16.0f;
	}
	else
	{
		std::shared_ptr<ColoredDrawable<Shaders::Phong>> cd = CommonUtility::singleton->createGameSphere(*mManipulator, mAmbientColor, this);
		mDrawables.emplace_back(cd);
	}
}

const Int FallingBubble::getType() const
{
	return GOT_FALLING_BUBBLE;
}

void FallingBubble::update()
{
	// Update motion
	if (mSpark)
	{
		if (wrapper.parameters.index >= wrapper.parameters.total)
		{
			destroyMe = true;
		}
		else
		{
			wrapper.parameters.index += mDeltaTime * wrapper.speed;
		}
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
	if (mSpark)
	{
		// Apply transformations
		const Matrix4 mat = Matrix4::translation(position) * Matrix4::scaling(Vector3(3.0f));
		mDrawables.at(0)->setTransformation(mat);
	}
	else
	{
		const Matrix4 mat = Matrix4::translation(position);
		mDrawables.at(0)->setTransformation(mat);
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mSpark)
	{
		((SpriteShader&)baseDrawable->getShader())
			.bindTexture(*baseDrawable->mTexture)
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.setColor(mAmbientColor)
			.setIndex(wrapper.parameters.index)
			.setTextureWidth(wrapper.parameters.texWidth)
			.setTextureHeight(wrapper.parameters.texHeight)
			.setRows(wrapper.parameters.rows)
			.setColumns(wrapper.parameters.columns)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPositions({ position + Vector3({ 0.0f, 40.0f, 5.0f }) })
			.setDiffuseColor(mDiffuseColor)
			.setAmbientColor(mAmbientColor)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.draw(*baseDrawable->mMesh);
	}
}

void FallingBubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

std::shared_ptr<TexturedDrawable<SpriteShader>> FallingBubble::createPlane(Object3D & parent, Resource<GL::Texture2D> & texture)
{
	Resource<GL::Mesh> resMesh{ CommonUtility::singleton->manager.get<GL::Mesh>(RESOURCE_MESH_PLANE) };

	if (!resMesh)
	{
		// Create test mesh
		Trade::MeshData meshData = Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates);

		GL::Buffer vertices;
		vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.textureCoordinates2DAsArray()));

		GL::Mesh mesh;
		mesh
			.setPrimitive(meshData.primitive())
			.setCount(meshData.vertexCount())
			.addVertexBuffer(std::move(vertices), 0, SpriteShader::Position{}, SpriteShader::TextureCoordinates{});

		// Add to resources
		CommonUtility::singleton->manager.set(resMesh.key(), std::move(mesh));
	}

	// Create shader
	Resource<GL::AbstractShaderProgram, SpriteShader> resShader{ CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, SpriteShader>(RESOURCE_SHADER_SPRITE) };
	
	if (!resShader)
	{
		// Create shader
		std::unique_ptr<GL::AbstractShaderProgram> shader = std::make_unique<SpriteShader>();

		// Add to resources
		Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
		CommonUtility::singleton->manager.set(resShader.key(), std::move(p));
	}

	// Create colored drawable
	std::shared_ptr<TexturedDrawable<SpriteShader>> td = std::make_shared<TexturedDrawable<SpriteShader>>(RoomManager::singleton->mDrawables, resShader, resMesh, texture);
	td->setParent(&parent);
	td->setDrawCallback(this);
	return td;
}