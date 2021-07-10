#include "StreamedAudioBuffer.h"

#include <string>
#include <Corrade/Utility/DebugStl.h>
#include <stb_vorbis.c>

std::unique_ptr<StreamedAudioBuffer> StreamedAudioBuffer::singleton = nullptr;

StreamedAudioBuffer::StreamedAudioBuffer()
{
	mBackBufferIndex = 0;
	mWasFed = false;
	mSeek = 0;
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
		stb_vorbis_close((stb_vorbis*)mStream);
		mStream = nullptr;
	}

	// Delete info struct
	if (mInfo != nullptr)
	{
		std::free(mInfo);
		mInfo = nullptr;
	}
}

void StreamedAudioBuffer::feed()
{
	// Check if back buffer was already fed
	if (mWasFed)
	{
		return;
	}

	// Check for stream validity
	if (mStream == nullptr)
	{
		Error{} << "Feeding is not possible, because stream is NULL.";
	}

	// Cast required pointers
	stb_vorbis* vs = (stb_vorbis*)mStream;
	stb_vorbis_info* vi = (stb_vorbis_info*)mInfo;

	// Work on back buffer
	std::size_t dbs = AS_BUFFER_SIZE * 2;
	const int amount = stb_vorbis_get_samples_short_interleaved(vs, vi->channels, mRawBuffers[mBackBufferIndex], dbs);
	if (amount > 0)
	{
		const Containers::ArrayView<const short> av{ mRawBuffers[mBackBufferIndex], dbs };
		mBuffers[mBackBufferIndex].setData(mCachedBufferFormat, av, mCachedSampleRate);

		mSeek += amount / 2;
		stb_vorbis_seek(vs, mSeek);
	}
	mWasFed = true;
}

void StreamedAudioBuffer::swapBuffers()
{
	mWasFed = false;
	mBackBufferIndex = 1 - mBackBufferIndex;
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

	mCachedNumberOfChannels = vi.channels;
	mCachedBufferFormat = computeBufferFormat();
	mCachedSampleRate = vi.sample_rate;

	// Feed back buffer
	feed();
}

Audio::Buffer& StreamedAudioBuffer::getFrontBuffer()
{
	return mBuffers[1 - mBackBufferIndex];
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

const Audio::BufferFormat StreamedAudioBuffer::computeBufferFormat() const
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