#pragma once

#include <thread>
#include <memory>
#include <Magnum/Magnum.h>

#include "../Common/CommonTypes.h"
#include "StreamedAudioBuffer.h"

using namespace Magnum;

class StreamedAudioPlayable
{
public:

	StreamedAudioPlayable(Object3D* object);
	~StreamedAudioPlayable();

	void clear();
	void loadAudio(const std::string & filename);
	std::unique_ptr<Audio::Playable3D>& playable();

protected:

	bool mLive;
	Object3D* mObject;

	std::unique_ptr<std::thread> mBgMusicThread;
	std::unique_ptr<StreamedAudioBuffer> mBgMusicStream;
	std::unique_ptr<Audio::Playable3D> mBgMusicPlayable;
};