#include "StreamedAudioPlayable.h"

#include <chrono>
#include <Corrade/Containers/ReferenceStl.h>

#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

using namespace std::chrono_literals;

StreamedAudioPlayable::StreamedAudioPlayable(Object3D* object) : mObject(object), mBufferIndex(0U), mLive(true)
{
	mLive = true;
	mThreadSleepDuration = 100ms;

	mState = Audio::Source::State::Initial;
	mGainLevel = 1.0f;
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
		// Stop source
		auto* source = &mPlayable->source();

		// Unqueue buffers
		{
			Containers::Reference<Audio::Buffer> av[2] = {
				std::ref(mBuffers[0]),
				std::ref(mBuffers[1])
			};
			source->unqueueBuffers({ av });
		}
		
		// De-reference playable
		mPlayable = nullptr;
	}
}

void StreamedAudioPlayable::loadAudio(const std::string & filename)
{
	// Create and load stream
	mStream = std::make_unique<StreamedAudioBuffer>();
	if (!mStream->openAudio(CommonUtility::singleton->mConfig.assetDir, filename))
	{
		Error{} << "Could not open audio for streaming" << filename;
		return;
	}

	// Compute sleep time for audio thread
	{
		const double bufSize = double(AS_BUFFER_SIZE);
		double factor = bufSize / double(mStream->getNumberOfChannels());
		factor *= 0.25;
		mThreadSleepDuration = std::chrono::milliseconds(Int(Math::ceil(factor / bufSize * 1000.0)));
	}

	// Create playable resource
	mPlayable = std::make_unique<Audio::Playable3D>(*mObject, &RoomManager::singleton->mAudioPlayables);

	// Create and detach thread
	mThread = std::make_unique<std::thread>([&]()
	{
		while (mLive)
		{
			// Check for playable
			if (mPlayable == nullptr)
			{
				std::this_thread::sleep_for(mThreadSleepDuration);
				continue;
			}

			// Check for source
			auto* source = getAvailableSource();
			if (source != nullptr)
			{
				// Set gain level
                if (source->gain() != mGainLevel)
                {
                    source->setGain(mGainLevel);
                }

				// Manage internal buffer queue
				if (mPlayable != nullptr)
				{
					// Check if audio buffer shall be feeded with another stream
					Int tryToFeed = 0;
					switch (mState)
					{
					case Audio::Source::State::Playing: {
						const auto internalState = source->state();
						if (internalState == Audio::Source::State::Playing)
						{
							tryToFeed = source->offsetInBytes() >= mBuffers[mBufferIndex].size() ? 1 : 0;
						}
						else if (internalState == Audio::Source::State::Stopped)
						{
							tryToFeed = 1;
						}
						else if (internalState == Audio::Source::State::Initial)
						{
							tryToFeed = 2;
						}
						break;
					};
					case Audio::Source::State::Paused: {
						if (source->state() != Audio::Source::State::Paused)
						{
							source->pause();
						}
						break;
					}
					case Audio::Source::State::Stopped: {
						if (source->state() != Audio::Source::State::Stopped)
						{
							source->stop();
						}
						break;
					}
					}

					// Feed N times (depending on desired state and internal state)
					for (Int i = 0; i < tryToFeed; ++i)
					{
						const int amount = mStream->feed();
						if (amount)
						{
							// Unqueue buffer to modify it
							{
								Containers::Reference<Audio::Buffer> av[1] = {
									std::ref(mBuffers[mBufferIndex])
								};
								source->unqueueBuffers({ av });
							}

							// Fill buffers as required
							{
								const short* rawBuffer = mStream->getRawBuffer();
								const Containers::ArrayView<const short> data{ rawBuffer, std::size_t(amount * mStream->getNumberOfChannels()) };
								mBuffers[mBufferIndex].setData(mStream->getBufferFormat(), data, mStream->getSampleRate());
							}

							// Re-enqueue buffer
							{
								Containers::Reference<Audio::Buffer> av[1] = {
									std::ref(mBuffers[mBufferIndex])
								};
								source->queueBuffers({ av });
							}

							// Switch buffer
							mBufferIndex = 1U - mBufferIndex;
						}

						// Play, if necessary
						if (source->state() != Audio::Source::State::Playing && mState == Audio::Source::State::Playing)
						{
							source->play();
						}
					}
				}
			}
			std::this_thread::sleep_for(mThreadSleepDuration);
		}

		// De-reference this thread
		mThread = nullptr;
	});
	mThread->detach();
}

const Audio::Source::State StreamedAudioPlayable::state() const
{
	return mState;
}

void StreamedAudioPlayable::play()
{
	mState = Audio::Source::State::Playing;

	auto* source = getAvailableSource();
	if (source != nullptr)
	{
		source->play();
	}
}

void StreamedAudioPlayable::pause()
{
	mState = Audio::Source::State::Paused;

	auto* source = getAvailableSource();
	if (source != nullptr)
	{
		source->pause();
	}
}

void StreamedAudioPlayable::stop()
{
	mState = Audio::Source::State::Stopped;

	auto* source = getAvailableSource();
	if (source != nullptr)
	{
		source->stop();
	}
}

const Float StreamedAudioPlayable::gain() const
{
	return mGainLevel;
}

void StreamedAudioPlayable::setGain(const Float level)
{
	mGainLevel = level;

	auto* source = getAvailableSource();
	if (source != nullptr)
	{
		source->setGain(mGainLevel);
	}
}

Audio::Source* StreamedAudioPlayable::getAvailableSource() const
{
	return mPlayable != nullptr ? &mPlayable->source() : nullptr;
}
