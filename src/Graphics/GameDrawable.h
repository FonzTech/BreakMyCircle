#pragma once

#include "../Common/CommonTypes.h"
#include "BaseDrawable.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

template <class ShaderType>
class GameDrawable : public BaseDrawable
{
public:
	template <class ShaderType>
	explicit GameDrawable(SceneGraph::DrawableGroup3D& group, const Resource<GL::AbstractShaderProgram, ShaderType>& shader) : BaseDrawable{ group }
	{
		mShader = shader;
		mDrawCallback = nullptr;
	}

	template <class ShaderType>
	explicit GameDrawable(SceneGraph::DrawableGroup3D& group, const Resource<GL::AbstractShaderProgram, ShaderType>& shader, const Resource<GL::Mesh>& mesh) : BaseDrawable{ group }
	{
		mShader = shader;
		mMesh = mesh;
		mDrawCallback = nullptr;
	}

	template <class ShaderType>
	explicit GameDrawable(SceneGraph::DrawableGroup3D& group, const Resource<GL::AbstractShaderProgram, ShaderType>& shader, const Resource<GL::Mesh>& mesh, const Resource<GL::Texture2D>& texture) : GameDrawable(group, shader, mesh)
	{
		mTexture = texture;
		mColor = Color4{ 1.0f };
	}

	template <class ShaderType>
	explicit GameDrawable(SceneGraph::DrawableGroup3D& group, const Resource<GL::AbstractShaderProgram, ShaderType>& shader, const Resource<GL::Mesh>& mesh, const Color4 color) : GameDrawable(group, shader, mesh)
	{
		mColor = color;
	}

	template <class ShaderType>
	explicit GameDrawable(const GameDrawable<ShaderType> * gd) : BaseDrawable{ *gd }
	{
		mShader = CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, ShaderType>(gd->mShader.key());
		mMesh = CommonUtility::singleton->manager.get<GL::Mesh>(gd->mMesh.key());
		mTexture = CommonUtility::singleton->manager.get<GL::Texture2D>(gd->mTexture.key());
		mColor = CommonUtility::singleton->manager.get<GL::Texture2D>(gd->mColor.key());
		mDrawCallback = gd->mDrawCallback;
	}

	ShaderType& getShader()
	{
		return *mShader;
	}

	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override
	{
		if (mDrawCallback != nullptr)
		{
			mDrawCallback->draw(this, transformationMatrix, camera);
			return;
		}

		((Shaders::Phong&) *mShader)
			.setAmbientColor(0x00000ff_rgbaf)
			.setSpecularColor(0xffffffff_rgbf)
			.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }))
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(mTexture, mTexture, nullptr, nullptr)
			.draw(*mMesh);
	}

	// Members
	Resource<GL::AbstractShaderProgram, ShaderType> mShader;
};