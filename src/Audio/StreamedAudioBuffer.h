#pragma once

#define AS_BUFFER_SIZE 44100

#include <unordered_map>
#include <Corrade/Containers/Optional.h>
#include <Magnum/Magnum.h>
#include <Magnum/Audio/Buffer.h>
#include <Magnum/Audio/BufferFormat.h>
#include <Magnum/Audio/Context.h>
#include <Magnum/Audio/Listener.h>
#include <Magnum/Audio/Playable.h>
#include <Magnum/Audio/PlayableGroup.h>
#include <Magnum/Audio/Source.h>

using namespace Magnum;

class StreamedAudioBuffer
{
public:

	StreamedAudioBuffer();
	~StreamedAudioBuffer();

	void openAudio(const std::string & basePath, const std::string & filename);
	Containers::Optional<Audio::Buffer> feed();

	const Audio::BufferFormat getBufferFormat() const;
	const Int getNumberOfChannels() const;
	const UnsignedInt getSampleRate() const;

protected:

	// STB Vorbis
	short mRawBuffer[AS_BUFFER_SIZE];
	void* mStream;
	void* mInfo;

	Audio::BufferFormat mCachedBufferFormat;
	Int mCachedNumberOfChannels;
	UnsignedInt mCachedSampleRate;

	// Methods
	void clear();
	const Audio::BufferFormat computeBufferFormat() const;
};