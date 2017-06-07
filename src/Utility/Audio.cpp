/*
The MIT License (MIT)

Copyright (c) 2016 Liav Turkia and Shahar Sandhaus

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <MACE/Utility/Audio.h>
#include <MACE/Core/System.h>

#include <iostream>
#include <cstdio>

namespace mc {

	AudioModule::AudioModule() {
		device = alcOpenDevice(NULL);
		context = alcCreateContext(device, NULL);
		alcMakeContextCurrent(context);
	}

	void AudioModule::init() {}

	void AudioModule::update() {}

	void AudioModule::destroy() {
		for( Index i = 0; i < sounds.size(); ++i ) {
			sounds[i].destroy();
		}

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}

	std::string AudioModule::getName() const {
		return "MACE/Audio";
	}

	void AudioModule::addSound(Sound & s) {
		sounds.push_back(s);

		s.init();
	}

	const std::vector<Sound>& AudioModule::getSounds() const {
		return sounds;
	}

	Sound::Sound(const std::string& p) :path(p) {}

	void Sound::init() {
		alGenBuffers(1, &buffer);
		alGenSources(1, &source);

		struct RIFF_Header {
			char chunkID[4];
			long chunkSize;//size not including chunkSize or chunkID
			char format[4];
		};

		struct WAVE_Format {
			char subChunkID[4];
			long subChunkSize;
			short audioFormat;
			short numChannels;
			long sampleRate;
			long byteRate;
			short blockAlign;
			short bitsPerSample;
		};

		struct WAVE_Data {
			char subChunkID[4]; //should contain the word data
			long subChunk2Size; //Stores the size of the data block
		};

		std::FILE* soundFile = NULL;
		WAVE_Format wave_format;
		RIFF_Header riff_header;
		WAVE_Data wave_data;

		try {
			soundFile = mc::os::fopen(&soundFile, path.c_str(), "rb");
			if( !soundFile )
				throw BadSoundError(path);

			if( !std::fread(&riff_header, sizeof(RIFF_Header), 1, soundFile) ) {
				throw BadSoundError("error loading WAVE data into struct!");
			}

			if( (riff_header.chunkID[0] != 'R' ||
			   riff_header.chunkID[1] != 'I' ||
			   riff_header.chunkID[2] != 'F' ||
			   riff_header.chunkID[3] != 'F') ||
			   (riff_header.format[0] != 'W' ||
			   riff_header.format[1] != 'A' ||
			   riff_header.format[2] != 'V' ||
			   riff_header.format[3] != 'E') ) {
				throw BadSoundError("Invalid RIFF or WAVE Header");
			}

			if( !std::fread(&wave_format, sizeof(WAVE_Format), 1, soundFile) ) {
				throw BadSoundError("error loading WAVE data into struct!");
			}

			if( wave_format.subChunkID[0] != 'f' ||
			   wave_format.subChunkID[1] != 'm' ||
			   wave_format.subChunkID[2] != 't' ||
			   wave_format.subChunkID[3] != ' ' ) {
				throw BadSoundError("Invalid Wave Format");
			}

			if( wave_format.subChunkSize > 16 ) {
				std::fseek(soundFile, sizeof(short), SEEK_CUR);
			}

			if( !std::fread(&wave_data, sizeof(WAVE_Data), 1, soundFile) ) {
				throw BadSoundError("error loading WAVE data into struct!");
			}

			if( wave_data.subChunkID[0] != 'd' ||
			   wave_data.subChunkID[1] != 'a' ||
			   wave_data.subChunkID[2] != 't' ||
			   wave_data.subChunkID[3] != 'a' ) {
				throw BadSoundError("Invalid data header");
			}

			buf = new unsigned char[wave_data.subChunk2Size];

			if( !std::fread(buf, wave_data.subChunk2Size, 1, soundFile) ) {
				throw BadSoundError("error loading WAVE data into struct!");
			}

			size = wave_data.subChunk2Size;
			frequency = wave_format.sampleRate;

			if( wave_format.numChannels == 1 ) {
				if( wave_format.bitsPerSample == 8 ) {
					format = AL_FORMAT_MONO8;
				} else if( wave_format.bitsPerSample == 16 ) {
					format = AL_FORMAT_MONO16;
				}
			} else if( wave_format.numChannels == 2 ) {
				if( wave_format.bitsPerSample == 8 ) {
					format = AL_FORMAT_STEREO8;
				} else if( wave_format.bitsPerSample == 16 ) {
					format = AL_FORMAT_STEREO16;
				}
			}

			alGenBuffers(1, &buffer);

			alBufferData(buffer, format, (void*) buf,
						 size, frequency);

			alGenSources(1, &source);
			alSourcei(source, AL_BUFFER, buffer);

			std::fclose(soundFile);
			return;
		} catch( const Error& error ) {

			std::cerr << error.what() << " : trying to load "
				<< path << std::endl;

			if( soundFile != nullptr ) {
				std::fclose(soundFile);
			}

			Error::handleError(error);
		}
	}

	void Sound::play() {
		alSourcePlay(source);
	}

	void Sound::pause() {
		alSourcePause(source);
	}

	void Sound::stop() {
		alSourceStop(source);
	}

	void Sound::destroy() {
		stop();

		alDeleteSources(1, &source);
		alDeleteBuffers(1, &buffer);
	}

	bool Sound::getProperty(const Byte param) const {
		return properties.getBit(param);
	}

	void Sound::setProperty(const Byte param, const bool value) {
		properties.setBit(param, value);
	}

	void Sound::setVolume(float vol) {
		volume = vol;
		alSourcef(source, AL_GAIN, volume);
	}

	void Sound::setLooping(const bool val) {
		properties.setBit(LOOPING, val);

		if( val ) {
			alSourcei(source, AL_LOOPING, AL_TRUE);
		} else {
			alSourcei(source, AL_LOOPING, val);
		}
	}

	bool Sound::isLooping() const {
		return properties.getBit(LOOPING);
	}

	void Sound::setProperties(const BitField & b) {
		properties = b;
	}

	BitField & Sound::getProperties() {
		return properties;
	}

	const BitField & Sound::getProperties() const {
		return properties;
	}
}//mc