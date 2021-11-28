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

	const Audio::Source::State state() const;
	void play();
	void pause();
	void stop();

	const Float gain() const;
	void setGain(const Float level);

protected:

	bool mLive;
	Object3D* mObject;

	Audio::Source::State mState;
	Float mGainLevel;

	UnsignedInt mBufferIndex;
	Audio::Buffer mBuffers[2];

	std::chrono::duration<Int, std::milli> mThreadSleepDuration;
	std::unique_ptr<std::thread> mThread;
	std::unique_ptr<StreamedAudioBuffer> mStream;
	std::unique_ptr<Audio::Playable3D> mPlayable;

	Audio::Source* getAvailableSource() const;
};