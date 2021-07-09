#pragma once

#define AS_BUFFER_SIZE 2048

#include <memory>
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

	Audio::Buffer& getBuffer();
	Audio::BufferFormat getBufferFormat();

protected:

	// STB Vorbis
	short mRawBuffer[AS_BUFFER_SIZE];
	void* mStream;
	void* mInfo;

	// Magnum audio
	Audio::Buffer mBuffer;

	// Methods
	void clear();
	void feed();
};