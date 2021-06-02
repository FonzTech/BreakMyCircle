#include "ColoredDrawable.h"

ColoredDrawable::ColoredDrawable(SceneGraph::DrawableGroup3D& group, const std::shared_ptr<GL::AbstractShaderProgram>& shader, const std::shared_ptr<GL::Mesh>& mesh, const Color4& color) : BaseDrawable{ group }
{
	mShader = shader;
	mMesh = mesh;
	mColor = color;
	mDrawCallback = nullptr;
}

void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mDrawCallback != nullptr)
	{
		mDrawCallback->draw(this, transformationMatrix, camera);
		return;
	}

	/*
	(*mShader.get())
		.setDiffuseColor(mColor)
		.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 1110.0f }))
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(*mMesh.get());
	*/
}