#include "FallingBubble.h"

#include <thread>

#include <Corrade/Containers/PointerStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>
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

std::shared_ptr<GameObject> FallingBubble::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

FallingBubble::FallingBubble(const Sint8 parentIndex, const Color3& ambientColor, const bool spark, const Float maxVerticalSpeed) : GameObject(parentIndex)
{
	// Assign members
	mAmbientColor = ambientColor;
	mSpark = spark;
	mVelocity = { 0.0f };
	mMaxVerticalSpeed = maxVerticalSpeed;

	mDelay = Float(std::rand() % 250) * 0.001f;

	// Create sparkle plane
	if (mSpark)
	{
		// Get sparkles texture
		Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SPARKLES);

		// Create plane
		std::shared_ptr<TexturedDrawable<SpriteShader>> td = createPlane(*mManipulator, resTexture);
		mDrawables.emplace_back(td);

		// Create shader data wrapper
		mWrapper.shader = &td->getShader();
		mWrapper.parameters.index = 0.0f;
		mWrapper.parameters.total = 16.0f;
		mWrapper.parameters.texWidth = Float(td->mTexture->imageSize(0).x());
		mWrapper.parameters.texHeight = Float(td->mTexture->imageSize(0).y());
		mWrapper.parameters.rows = 4.0f;
		mWrapper.parameters.columns = 4.0f;
		mWrapper.speed = 16.0f;
	}
	else
	{
		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
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
		if (mWrapper.parameters.index >= mWrapper.parameters.total)
		{
			destroyMe = true;
		}
		else
		{
			mWrapper.parameters.index += mDeltaTime * mWrapper.speed;
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
			if (mVelocity.y() > mMaxVerticalSpeed)
			{
				mVelocity += { 0.0f, -2.0f, 0.0f };
			}
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
			.setIndex(mWrapper.parameters.index)
			.setTextureWidth(mWrapper.parameters.texWidth)
			.setTextureHeight(mWrapper.parameters.texHeight)
			.setRows(mWrapper.parameters.rows)
			.setColumns(mWrapper.parameters.columns)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPosition(position + Vector3(0.0f, 0.0f, 1.0f))
			.setLightColor(0xffffff60_rgbaf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0x808080_rgbf)
			.setDiffuseColor(0x808080_rgbf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
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
	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	std::shared_ptr<TexturedDrawable<SpriteShader>> td = std::make_shared<TexturedDrawable<SpriteShader>>(*drawables, resShader, resMesh, texture);
	td->setParent(&parent);
	td->setDrawCallback(this);
	return td;
}