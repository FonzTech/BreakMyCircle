#include "BaseDrawable.h"

BaseDrawable::BaseDrawable(SceneGraph::DrawableGroup3D& group) : SceneGraph::Drawable3D{ *this, &group }
{
}

BaseDrawable::BaseDrawable(const BaseDrawable & d) : SceneGraph::Drawable3D{ *this, const_cast<SceneGraph::DrawableGroup3D*>(d.drawables()) }
{
}

void BaseDrawable::setDrawCallback(IDrawCallback* drawCallback)
{
	mDrawCallback = drawCallback;
}

UnsignedInt BaseDrawable::getObjectId() const
{
	return mObjectId;
}

const void BaseDrawable::setObjectId(const UnsignedInt objectId)
{
	mObjectId = objectId;
}

const void BaseDrawable::pushToFront()
{
	(*group())
		.remove(*this)
		.add(*this);
}