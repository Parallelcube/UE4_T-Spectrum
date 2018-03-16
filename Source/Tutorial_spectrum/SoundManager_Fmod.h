#ifndef SoundManager_Fmod_H
#define SoundManager_Fmod_H

#include <string>
#include <vector>

namespace FMOD 
{
	class System;
	class Channel;
	class Sound;
	class DSP;
};

class SoundManager_Fmod
{
public:
	SoundManager_Fmod();
	~SoundManager_Fmod();

	int initialize();
	int loadSoundFromPath(std::string pathToFile);
	int loadSoundFromMemory(char* memoryPtr, unsigned int memorySize);
	void playSound();
	void pauseSound(bool unPause = false);

	void update();
	int initializeSpectrum_Linear(int maxBands);
	int initializeSpectrum_Log(int bandsPerOctave);
	void getSpectrum_Linear(float* spectrum);
	void getSpectrum_Log(float* spectrum);
		
private:
	FMOD::System* _system;
	FMOD::Channel* _channel;
	FMOD::Sound* _sound;

	FMOD::DSP*	_dsp;
	int _windowSize;

	std::vector<int> _numSamplesPerBar_linear;
	std::vector<int> _numSamplesPerBar_log;
};

#endif
