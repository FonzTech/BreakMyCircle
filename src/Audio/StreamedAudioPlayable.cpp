#include "StreamedAudioPlayable.h"

#include <chrono>

#include "../RoomManager.h"

using namespace std::chrono_literals;

StreamedAudioPlayable::StreamedAudioPlayable(Object3D* object) : mObject(object)
{
	mLive = true;
}

StreamedAudioPlayable::~StreamedAudioPlayable()
{
	mBgMusicStream = nullptr;
	mBgMusicThread = nullptr;
	mBgMusicPlayable = nullptr;
}

void StreamedAudioPlayable::clear()
{
	// Make music thread terminate
	mLive = false;

	// Wait until the thread finishes
	while (mBgMusicThread != nullptr);

	// Remove references to buffer and playable object
	if (mBgMusicPlayable != nullptr)
	{
		mBgMusicPlayable->source()
			.stop()
			.setBuffer(nullptr);

		mBgMusicPlayable = nullptr;
	}
}

void StreamedAudioPlayable::loadAudio(const std::string & filename)
{
	// Create and load stream
	mBgMusicStream = std::make_unique<StreamedAudioBuffer>();
	mBgMusicStream->openAudio(filename);

	// Create and detach thread
	mBgMusicThread = std::make_unique<std::thread>([&]()
	{
		const Int limit = AS_BUFFER_SIZE / mBgMusicStream->getNumberOfChannels() - 256;
		while (mLive)
		{
			if (mBgMusicPlayable == nullptr)
			{
				continue;
			}

			auto& source = mBgMusicPlayable->source();
			const bool b1 = source.state() == Audio::Source::State::Stopped;
			const bool b2 = source.state() == Audio::Source::State::Playing && source.offsetInSamples() >= limit;
			if (b1 || b2) 
			{
				if (b2)
				{
					source.stop();
				}

				source.setBuffer(nullptr);

				if (mBgMusicPlayable != nullptr)
				{
					mBgMusicStream->feed();
					source.setBuffer(&mBgMusicStream->getFrontBuffer());
					source.setOffsetInSamples(0);
					source.play();
				}
			}
			std::this_thread::sleep_for(20ms);
		}
		mBgMusicThread = nullptr;
	});
	mBgMusicThread->detach();

	// Create playable resource with buffer
	mBgMusicPlayable = std::make_unique<Audio::Playable3D>(*mObject, &RoomManager::singleton->mAudioPlayables);
	mBgMusicPlayable->source()
		.setBuffer(&mBgMusicStream->getFrontBuffer())
		.setMinGain(1.0f)
		.setMaxGain(1.0f)
		.setLooping(false)
		.play();

}

std::unique_ptr<Audio::Playable3D>& StreamedAudioPlayable::playable()
{
	return mBgMusicPlayable;
}