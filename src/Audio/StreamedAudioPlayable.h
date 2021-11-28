#pragma once

#include <thread>
#include <memory>
#include <Corrade/Containers/Reference.h>
#include <Magnum/Magnum.h>

#include "../Common/CommonTypes.h"
#include "StreamedAudioBuffer.h"

using namespace Magnum;

class StreamedAudioPlayable
{
public:

	StreamedAudioPlayable(Object3D* object);

	/**
		The `clear` instance method must be called first, otherwise null
		pointer exception will occur inside the other thread.
	*/
	~StreamedAudioPlayable();

	void clear();
	void loadAudio(const std::string & filename);
	std::unique_ptr<Audio::Playable3D>& playable();

protected:

	bool mLive;
	Object3D* mObject;

	Int mBufferIndex;
	Audio::Buffer mRawBuffers[2];
	Containers::Reference<Audio::Buffer> mBuffers[2][1];

	std::unique_ptr<std::thread> mThread;
	std::unique_ptr<StreamedAudioBuffer> mStream;
	std::unique_ptr<Audio::Playable3D> mPlayable;
};