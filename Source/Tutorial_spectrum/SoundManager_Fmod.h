#ifndef SoundManager_Fmod_H
#define SoundManager_Fmod_H

#include <string>
#include <vector>
#include <deque>

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
	void initializeBeatDetector();
	void getSpectrum_Linear(float* spectrum, float* averageSpectrum);
	void getSpectrum_Log(float* spectrum, float* averageSpectrum);
	void getBeat(float* spectrum, float* averageSpectrum, bool& isBass, bool& isLowM);

private:
	FMOD::System* _system;
	FMOD::Channel* _channel;
	FMOD::Sound* _sound;

	FMOD::DSP* _dsp;
	int _windowSize;
	float _samplingFrequency;

	std::vector<int> _numSamplesPerBar_linear;
	std::vector<int> _numSamplesPerBar_log;

	typedef std::deque<std::vector<float> > FFTHistoryContainer;

	int _FFThistory_MaxSize;
	std::vector<int> _beatDetector_bandLimits;
	FFTHistoryContainer _FFTHistory_linear;
	FFTHistoryContainer _FFTHistory_log;
	FFTHistoryContainer _FFTHistory_beatDetector;

	static void fillAverageSpectrum(float* averageSpectrum, int numBands, const FFTHistoryContainer& fftHistory);
	static void fillVarianceSpectrum(float* varianceSpectrum, int numBands, const FFTHistoryContainer& fftHistory, const float* averageSpectrum);
	static float beatThreshold(float variance);
};

#endif
