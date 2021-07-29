#pragma once

#include "../Common/CommonTypes.h"
#include "BaseDrawable.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

template <class ShaderType>
class TexturedDrawable : public BaseDrawable
{
public:
	template <class ShaderType>
	explicit TexturedDrawable(SceneGraph::DrawableGroup3D& group, const Resource<GL::AbstractShaderProgram, ShaderType>& shader, const Resource<GL::Mesh>& mesh, const Resource<GL::Texture2D>& texture) : BaseDrawable{ group }
	{
		mShader = shader;
		mMesh = mesh;
		mTexture = texture;
		mDrawCallback = nullptr;
	}

	template <class ShaderType>
	explicit TexturedDrawable(const TexturedDrawable<ShaderType> * td) : BaseDrawable{ *td }
	{
		mShader = CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, ShaderType>(td->mShader.key());
		mMesh = CommonUtility::singleton->manager.get<GL::Mesh>(td->mMesh.key());
		mTexture = CommonUtility::singleton->manager.get<GL::Texture2D>(td->mTexture.key());
		mDrawCallback = td->mDrawCallback;
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