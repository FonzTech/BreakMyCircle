#pragma once

#include "CommonTypes.h"
#include "BaseDrawable.h"

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

	ShaderType& getShader()
	{
		return *mShader;
	}

protected:
	Resource<GL::AbstractShaderProgram, ShaderType> mShader;

	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override
	{
		if (mDrawCallback != nullptr)
		{
			mDrawCallback->draw(this, transformationMatrix, camera);
			return;
		}

		((Shaders::Phong&) *mShader)
			.setAmbientColor(0xffffffff_rgbaf)
			.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }))
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindDiffuseTexture(*mTexture)
			.draw(*mMesh);
	}
};