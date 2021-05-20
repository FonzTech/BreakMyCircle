#include "BaseDrawable.h"

BaseDrawable::BaseDrawable(SceneGraph::DrawableGroup3D& group) : SceneGraph::Drawable3D{ *this, &group }
{
}

void BaseDrawable::setDrawCallback(IDrawCallback* drawCallback)
{
	mDrawCallback = drawCallback;
}