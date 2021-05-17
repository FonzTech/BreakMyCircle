#include "TexturedDrawable.h"

TexturedDrawable::TexturedDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, GL::Texture2D& texture) : SceneGraph::Drawable3D{ *this, &group }, mShader(shader), mMesh(mesh), mTexture(texture)
{
}

void TexturedDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	mShader.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }));
	mShader.setTransformationMatrix(transformationMatrix);
	mShader.setNormalMatrix(transformationMatrix.normalMatrix());
	mShader.setProjectionMatrix(camera.projectionMatrix());
	mShader.bindDiffuseTexture(mTexture);
	mShader.draw(mMesh);
}