#include "StreamedAudioPlayable.h"

#include <chrono>

#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

using namespace std::chrono_literals;

StreamedAudioPlayable::StreamedAudioPlayable(Object3D* object) : mObject(object)
{
	mLive = true;
}

StreamedAudioPlayable::~StreamedAudioPlayable()
{
	mStream = nullptr;
	mThread = nullptr;
	mPlayable = nullptr;
}

void StreamedAudioPlayable::clear()
{
	// Make music thread terminate
	mLive = false;

	// Wait until the thread finishes
	while (mThread != nullptr);

	// Remove references to buffer and playable object
	if (mPlayable != nullptr)
	{
		mPlayable->source()
			.stop()
			.setBuffer(nullptr);

		mPlayable = nullptr;
	}
}

void StreamedAudioPlayable::loadAudio(const std::string & filename)
{
	// Create and load stream
	mStream = std::make_unique<StreamedAudioBuffer>();
	mStream->openAudio(CommonUtility::singleton->mAssetDir, filename);

	// Create and detach thread
	mThread = std::make_unique<std::thread>([&]()
	{
		const Int limit = AS_BUFFER_SIZE / mStream->getNumberOfChannels() - 256;
		while (mLive)
		{
			if (mPlayable == nullptr)
			{
				continue;
			}

			auto& source = mPlayable->source();
			const bool b1 = source.state() == Audio::Source::State::Stopped;
			const bool b2 = source.state() == Audio::Source::State::Playing && source.offsetInSamples() >= limit;
			if (b1 || b2) 
			{
				if (b2)
				{
					source.stop();
				}

				source.setBuffer(nullptr);

				if (mPlayable != nullptr)
				{
					mStream->feed();
					source.setBuffer(&mStream->getFrontBuffer());
					source.setOffsetInSamples(0);
					source.play();
				}
			}
			std::this_thread::sleep_for(20ms);
		}
		mThread = nullptr;
	});
	mThread->detach();

	// Create playable resource with buffer
	mPlayable = std::make_unique<Audio::Playable3D>(*mObject, &RoomManager::singleton->mAudioPlayables);
	mPlayable->source()
		.setBuffer(&mStream->getFrontBuffer())
		.setLooping(false)
		.play();

}

std::unique_ptr<Audio::Playable3D>& StreamedAudioPlayable::playable()
{
	return mPlayable;
}