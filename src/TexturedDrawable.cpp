#include "TexturedDrawable.h"

TexturedDrawable::TexturedDrawable(SceneGraph::DrawableGroup3D& group, const std::shared_ptr<Shaders::Phong>& shader, const std::shared_ptr<GL::Mesh>& mesh, const std::shared_ptr<GL::Texture2D>& texture) : BaseDrawable{ group }
{
	mShader = shader;
	mMesh = mesh;
	mTexture = texture;
	mDrawCallback = nullptr;
}

void TexturedDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mDrawCallback != nullptr)
	{
		mDrawCallback->draw(this, transformationMatrix, camera);
		return;
	}
	(*mShader.get())
		.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }))
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindDiffuseTexture(*mTexture.get())
		.draw(*mMesh.get());
}