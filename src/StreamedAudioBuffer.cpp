#include "StreamedAudioBuffer.h"

#include <string>
#include <Corrade/Utility/DebugStl.h>
#include <stb_vorbis.c>

std::unique_ptr<StreamedAudioBuffer> StreamedAudioBuffer::singleton = nullptr;

StreamedAudioBuffer::StreamedAudioBuffer()
{
}

StreamedAudioBuffer::~StreamedAudioBuffer()
{
	clear();
}

void StreamedAudioBuffer::clear()
{
	if (mStream != nullptr)
	{
		stb_vorbis_close((stb_vorbis*)mStream);
		mStream = nullptr;
	}

	if (mInfo != nullptr)
	{
		std::free(mInfo);
		mInfo = nullptr;
	}
}

void StreamedAudioBuffer::feed()
{
	if (mStream == nullptr)
	{
		Error{} << "Feeding is not possible, because stream is NULL.";
	}

	stb_vorbis* vs = (stb_vorbis*) mStream;
	stb_vorbis_info* vi = (stb_vorbis_info*) mInfo;

	const int amount = stb_vorbis_get_samples_short_interleaved(vs, vi->channels, mRawBuffer, AS_BUFFER_SIZE);
	if (amount > 0)
	{
		const Containers::ArrayView<const short> av = mRawBuffer;
		const Audio::BufferFormat bf = getBufferFormat();
		mBuffer.setData(bf, av, vi->sample_rate);
	}
}

void StreamedAudioBuffer::openAudio(const std::string & filename)
{
	// Open stream
	{
		std::string s = "audios/" + filename + ".ogg";
		mStream = stb_vorbis_open_filename(s.c_str(), nullptr, nullptr);
	}

	if (mStream == nullptr)
	{
		Error{} << "Could not open file" << filename << "for streaming.";
		return;
	}

	// Cast to specialized type
	stb_vorbis* vs = (stb_vorbis*) mStream;

	// Get info and copy it to heap
	stb_vorbis_info vi = stb_vorbis_get_info(vs);
	mInfo = std::malloc(sizeof(stb_vorbis_info));
	if (mInfo == nullptr)
	{
		Error{} << "Could not malloc a struct of size stb_vorbis_info.";
		clear();
		return;
	}

	std::memcpy(mInfo, &vi, sizeof(stb_vorbis_info));

	// Work on stream
	uint32 samples = stb_vorbis_stream_length_in_samples(vs) * vi.channels;

	Debug{} << "Channels:" << vi.channels;
	Debug{} << "Sample rate:" << vi.sample_rate;
	Debug{} << "Samples:" << samples;
	Debug{} << "Duration:" << stb_vorbis_stream_length_in_seconds(vs) << "seconds.";
	
	feed();
}

Audio::Buffer& StreamedAudioBuffer::getBuffer()
{
	return mBuffer;
}

Audio::BufferFormat StreamedAudioBuffer::getBufferFormat()
{
	stb_vorbis_info* vi = (stb_vorbis_info*) mInfo;

	if (vi->channels == 1)
	{
		return Audio::BufferFormat::Mono16;
	}
	else if (vi->channels == 2)
	{
		return Audio::BufferFormat::Stereo16;
	}
	else if (vi->channels == 4)
	{
		return Audio::BufferFormat::Quad16;
	}
	else if (vi->channels == 6)
	{
		return Audio::BufferFormat::Surround51Channel16;
	}
	else if (vi->channels == 7)
	{
		return Audio::BufferFormat::Surround61Channel16;
	}
	else if (vi->channels == 8)
	{
		return Audio::BufferFormat::Surround71Channel16;
	}

	Error() << "StreamedAudioBuffer: unsupported channel count" << vi->channels << "with" << 16 << "bits per sample";
	return Audio::BufferFormat::Mono8;
}