#pragma once

#define AS_BUFFER_SIZE 4096
#define AS_BUFFER_SIZE_HALF AS_BUFFER_SIZE / 2

#include <memory>
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

	static std::unique_ptr<StreamedAudioBuffer> singleton;

	StreamedAudioBuffer();
	~StreamedAudioBuffer();

	void openAudio(const std::string & filename);

	Audio::Buffer& getFrontBuffer();
	const Audio::BufferFormat getBufferFormat() const;
	const Int getNumberOfChannels() const;
	const UnsignedInt getSampleRate() const;
	void feed();

protected:

	// STB Vorbis
	short mRawBuffers[AS_BUFFER_SIZE][2];
	void* mStream;
	void* mInfo;

	// Magnum audio
	Audio::Buffer mBuffers[2];

	Audio::BufferFormat mCachedBufferFormat;
	Int mCachedNumberOfChannels;
	UnsignedInt mCachedSampleRate;

	// Methods
	void clear();
	const Audio::BufferFormat computeBufferFormat() const;
};