#include "StreamedAudioBuffer.h"

#include <string>
#include <Corrade/Utility/DebugStl.h>

#include <stb_vorbis_prefix.c>

StreamedAudioBuffer::StreamedAudioBuffer()
{
}

StreamedAudioBuffer::~StreamedAudioBuffer()
{
	clear();
}

void StreamedAudioBuffer::clear()
{
	// Close stream
	if (mStream != nullptr)
	{
		stb_vorbis_fix_close((stb_vorbis_fix*)mStream);
		mStream = nullptr;
	}

	// Delete info struct
	if (mInfo != nullptr)
	{
		std::free(mInfo);
		mInfo = nullptr;
	}
}

const int StreamedAudioBuffer::feed()
{
	// Check for stream validity
	if (mStream == nullptr)
	{
        Error{} << "Feeding is not possible, because stream is NULL.";
	}

	// Cast required pointers
	stb_vorbis_fix* vs = (stb_vorbis_fix*)mStream;
	stb_vorbis_fix_info* vi = (stb_vorbis_fix_info*)mInfo;

	// Work on back buffer
	for (UnsignedInt i = 0; i < 2; ++i)
	{
		const int amount = stb_vorbis_fix_get_samples_short_interleaved(vs, vi->channels, mRawBuffer, AS_BUFFER_SIZE);
		if (amount > 0)
		{
			return amount;
		}
		else if (i)
		{
			Error{} << "Buffer appear to be empty. Seek to the beginning was useless.";
		}
		else
		{
			stb_vorbis_fix_seek(vs, 0);
		}
	}

	return 0;
}

bool StreamedAudioBuffer::openAudio(const std::string & basePath, const std::string & filename)
{
	// Open stream
	{
		std::string s = basePath + "audios/" + filename + ".ogg";
		mStream = stb_vorbis_fix_open_filename(s.c_str(), nullptr, nullptr);
	}

	if (mStream == nullptr)
	{
		Error{} << "Could not open file" << filename << "for streaming.";
		return false;
	}

	// Cast to specialized type
	stb_vorbis_fix* vs = (stb_vorbis_fix*)mStream;

	// Get info and copy it to heap
	stb_vorbis_fix_info vi = stb_vorbis_fix_get_info(vs);
	mInfo = std::malloc(sizeof(stb_vorbis_fix_info));
	if (mInfo == nullptr)
	{
		Error{} << "Could not malloc a struct of size stb_vorbis_info.";
		clear();
		return false;
	}

	std::memcpy(mInfo, &vi, sizeof(stb_vorbis_fix_info));

	// Work on stream
	auto samples = stb_vorbis_fix_stream_length_in_samples(vs) * vi.channels;

	Debug{} << "Channels:" << vi.channels;
	Debug{} << "Sample rate:" << vi.sample_rate;
	Debug{} << "Samples:" << samples;
	Debug{} << "Duration:" << stb_vorbis_fix_stream_length_in_seconds(vs) << "seconds.";

	mCachedNumberOfChannels = vi.channels;
	mCachedBufferFormat = computeBufferFormat();
	mCachedSampleRate = vi.sample_rate;

	return true;
}

const Audio::BufferFormat StreamedAudioBuffer::getBufferFormat() const
{
	return mCachedBufferFormat;
}

const Int StreamedAudioBuffer::getNumberOfChannels() const
{
	return mCachedNumberOfChannels;
}

const UnsignedInt StreamedAudioBuffer::getSampleRate() const
{
	return mCachedSampleRate;
}

const short* StreamedAudioBuffer::getRawBuffer() const
{
	return mRawBuffer;
}

const Audio::BufferFormat StreamedAudioBuffer::computeBufferFormat() const
{
	stb_vorbis_fix_info* vi = (stb_vorbis_fix_info*)mInfo;

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
