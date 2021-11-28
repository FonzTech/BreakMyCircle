#include "StreamedAudioPlayable.h"

#include <chrono>
#include <Corrade/Containers/ReferenceStl.h>

#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

using namespace std::chrono_literals;

StreamedAudioPlayable::StreamedAudioPlayable(Object3D* object) : mObject(object), mBufferIndex(-1), mBuffers{ mRawBuffers[0], mRawBuffers[1] }, mLive(true)
{
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
			.stop();

		mPlayable = nullptr;
	}
}

void StreamedAudioPlayable::loadAudio(const std::string & filename)
{
	// Create and load stream
	mStream = std::make_unique<StreamedAudioBuffer>();
	mStream->openAudio(CommonUtility::singleton->mConfig.assetDir, filename);

	// Create playable resource
	mPlayable = std::make_unique<Audio::Playable3D>(*mObject, &RoomManager::singleton->mAudioPlayables);

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
			const bool b1 = source.state() == Audio::Source::State::Initial;
			// const bool b2 = source.state() == Audio::Source::State::Playing && source.offsetInSamples() >= limit;
			const bool b2 = false;
			const bool b3 = source.state() == Audio::Source::State::Stopped;
			if (b1 || b2 || b3)
			{
				if (mPlayable != nullptr)
				{
					// Check if another buffer is available
					Containers::Optional<Audio::Buffer> buffer = mStream->feed();
					if (buffer != Containers::NullOpt)
					{
						// Dequeue old buffer
						if (mBufferIndex == -1)
						{
							mBufferIndex = 0;
						}
						else
						{
							// Unqueue old buffer
							source.unqueueBuffers({ mBuffers[mBufferIndex] });

							// Switch to the other buffer index
							mBufferIndex = 1 - mBufferIndex;
						}

						// Fill "back" buffer
						mRawBuffers[mBufferIndex] = std::move(*buffer);
						mBuffers[mBufferIndex][0] = std::ref(mRawBuffers[mBufferIndex]);

						// Enqueue into source
						source.queueBuffers({ mBuffers[mBufferIndex] });

						// Play if necessary
						if (b1 || b3)
						{
							source.play();
						}
					}
				}
			}
			std::this_thread::sleep_for(200ms);
		}
		mThread = nullptr;
	});
	mThread->detach();

}

std::unique_ptr<Audio::Playable3D>& StreamedAudioPlayable::playable()
{
	return mPlayable;
}