#include "SoundManager_Fmod.h"

#include "fmod.hpp"

#include <string>
#include <math.h>

using namespace std;

SoundManager_Fmod::SoundManager_Fmod():_system(NULL), _channel(NULL), _sound(NULL), _dsp(NULL), _windowSize(1024), _samplingFrequency(0), _FFThistory_MaxSize(0)
{
}

int SoundManager_Fmod::initialize()
{
	FMOD_RESULT result = FMOD::System_Create(&_system);
	if (result != FMOD_OK)
	{
		return result;
	}
	else
	{
		_system->init(1, FMOD_INIT_NORMAL, NULL);

		_system->createDSPByType(FMOD_DSP_TYPE_FFT, &_dsp);
		_dsp->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_TRIANGLE);
		_dsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, _windowSize);
	}

	return 0;
}

SoundManager_Fmod::~SoundManager_Fmod()
{
	if (_sound)
	{
		_channel->removeDSP(_dsp);
		_sound->release();
	}

	if (_system)
	{
		_dsp->release();
		_system->close();
		_system->release();
	}
}

int SoundManager_Fmod::loadSoundFromPath(std::string pathToFile)
{
	FMOD_RESULT result = _system->createSound(pathToFile.c_str(), FMOD_LOOP_NORMAL | FMOD_CREATESAMPLE, 0, &_sound);
	return result;
}

int SoundManager_Fmod::loadSoundFromMemory(char* memoryPtr, unsigned int memorySize)
{
	FMOD_CREATESOUNDEXINFO sndinfo = { 0 };
	sndinfo.cbsize = sizeof(sndinfo);
	sndinfo.length = memorySize;

	FMOD_RESULT result = _system->createSound(memoryPtr, FMOD_OPENMEMORY | FMOD_LOOP_NORMAL | FMOD_CREATESAMPLE, &sndinfo, &_sound);
	return result;
}

void SoundManager_Fmod::playSound()
{
	_FFTHistory_linear.clear();
	_FFTHistory_log.clear();

	_system->playSound(_sound, 0, false, &_channel);

	_channel->getFrequency(&_samplingFrequency);
	_FFThistory_MaxSize = _samplingFrequency / _windowSize;

	_channel->addDSP(0, _dsp);
	_dsp->setActive(true);
}

void SoundManager_Fmod::pauseSound(bool unPause)
{
	bool isPaused;
	_channel->getPaused(&isPaused);
	if (isPaused && unPause)
	{
		_channel->setPaused(false);
	}
	else if(!isPaused && !unPause)
	{
		_channel->setPaused(true);
	}
}

void SoundManager_Fmod::update()
{
	_system->update();
}

int SoundManager_Fmod::initializeSpectrum_Linear(int maxBands)
{
	int barSamples = (_windowSize / 2) / maxBands;

	//calculates num fft samples per bar
	_numSamplesPerBar_linear.clear();
	for (int i = 0; i < maxBands; ++i)
	{
		_numSamplesPerBar_linear.push_back(barSamples);
	}
	return _numSamplesPerBar_linear.size(); //effectiveBars
}

void SoundManager_Fmod::getSpectrum_Linear(float* spectrum, float* averageSpectrum)
{
	FMOD_DSP_PARAMETER_FFT* dspFFT = NULL;
	FMOD_RESULT result = _dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void **)&dspFFT, 0, 0, 0);
	if (dspFFT)
	{
		// Only read / display half of the buffer typically for analysis 
		// as the 2nd half is usually the same data reversed due to the nature of the way FFT works.
		int length = dspFFT->length / 2;
		int numChannels = dspFFT->numchannels;

		if (length > 0)
		{
			//Spectrum NOW
			int indexFFT = 0;
			for (int index = 0; index < _numSamplesPerBar_linear.size(); ++index)
			{
				for (int frec = 0; frec < _numSamplesPerBar_linear[index]; ++frec)
				{
					for (int channel = 0; channel < numChannels; ++channel)
					{
						spectrum[index] += dspFFT->spectrum[channel][indexFFT];
					}
					++indexFFT;
				}
				spectrum[index] /= (float)(_numSamplesPerBar_linear[index] * numChannels);
			}

			//Average Spectrum (1 second)
			std::vector<float> fftResult;
			fftResult.reserve(_numSamplesPerBar_linear.size());
			for (int index = 0; index < _numSamplesPerBar_linear.size(); ++index)
			{
				fftResult.push_back(spectrum[index]);
			}

			if (_FFTHistory_linear.size() >= _FFThistory_MaxSize)
			{
				_FFTHistory_linear.pop_front();
			}

			_FFTHistory_linear.push_back(fftResult);


			fillAverageSpectrum(averageSpectrum, _numSamplesPerBar_linear.size(), _FFTHistory_linear);
		}
	}
}

int SoundManager_Fmod::initializeSpectrum_Log(int maxBars)
{
	//calculates octave frequency
	std::vector<int> frequencyOctaves;
	frequencyOctaves.push_back(0);
	for (int i = 1; i < 13; ++i)
	{
		frequencyOctaves.push_back((int)((44100 / 2) / (float)pow(2, 12 - i)));
	}

	int bandWidth = (44100 / _windowSize);
	int bandsPerOctave = maxBars / 12; //octaves

	//calculates num fft samples per bar
	_numSamplesPerBar_log.clear();
	for (int octave = 0; octave < 12; ++octave)
	{
		int indexLow = frequencyOctaves[octave] / bandWidth;
		int indexHigh = (frequencyOctaves[octave + 1]) / bandWidth;
		int octavaIndexes = (indexHigh - indexLow);

		if (octavaIndexes > 0)
		{
			if (octavaIndexes <= bandsPerOctave)
			{
				for (int count = 0; count < octavaIndexes; ++count)
				{
					_numSamplesPerBar_log.push_back(1);
				}
			}
			else
			{
				for (int count = 0; count < bandsPerOctave; ++count)
				{
					_numSamplesPerBar_log.push_back(octavaIndexes / bandsPerOctave);
				}
			}
		}
	}

	return _numSamplesPerBar_log.size(); //effectiveBars
}

void SoundManager_Fmod::getSpectrum_Log(float* spectrum, float* averageSpectrum)
{
	FMOD_DSP_PARAMETER_FFT* dspFFT = NULL;
	FMOD_RESULT result = _dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void **)&dspFFT, 0, 0, 0);
	if (dspFFT)
	{
		// Only read / display half of the buffer typically for analysis 
		// as the 2nd half is usually the same data reversed due to the nature of the way FFT works.
		int length = dspFFT->length / 2;
		int numChannels = dspFFT->numchannels;

		if (length > 0)
		{
			//Spectrum NOW
			int indexFFT = 0;
			for (int index = 0; index < _numSamplesPerBar_log.size(); ++index)
			{
				for (int frec = 0; frec < _numSamplesPerBar_log[index]; ++frec)
				{
					for (int channel = 0; channel < numChannels; ++channel)
					{
						spectrum[index] += dspFFT->spectrum[channel][indexFFT];
					}
					++indexFFT;
				}
				spectrum[index] /= (float)(_numSamplesPerBar_log[index] * numChannels);
			}

			//Average Spectrum (1 second)
			std::vector<float> fftResult;
			fftResult.reserve(_numSamplesPerBar_log.size());
			for (int index = 0; index < _numSamplesPerBar_log.size(); ++index)
			{
				fftResult.push_back(spectrum[index]);
			}

			if (_FFTHistory_log.size() >= _FFThistory_MaxSize)
			{
				_FFTHistory_log.pop_front();
			}

			_FFTHistory_log.push_back(fftResult);


			fillAverageSpectrum(averageSpectrum, _numSamplesPerBar_log.size(), _FFTHistory_log);
		}
	}
}

void SoundManager_Fmod::fillAverageSpectrum(float* averageSpectrum, int numBands, const FFTHistoryContainer& fftHistory)
{
	for (FFTHistoryContainer::const_iterator fftResult_it = fftHistory.cbegin();
		fftResult_it != fftHistory.cend();
		++fftResult_it)
	{
		const std::vector<float>& fftResult = *fftResult_it;

		for (int index = 0; index < fftResult.size(); ++index)
		{
			averageSpectrum[index] += fftResult[index];
		}
	}

	for (int index = 0; index < numBands; ++index)
	{
		averageSpectrum[index] /= (fftHistory.size());
	}
}

void SoundManager_Fmod::fillVarianceSpectrum(float* varianceSpectrum, int numBands, const FFTHistoryContainer& fftHistory, const float* averageSpectrum)
{
	for (FFTHistoryContainer::const_iterator fftResult_it = fftHistory.cbegin();
		fftResult_it != fftHistory.cend();
		++fftResult_it)
	{
		const std::vector<float>& fftResult = *fftResult_it;

		for (int index = 0; index < fftResult.size(); ++index)
		{
			varianceSpectrum[index] += (fftResult[index] - averageSpectrum[index]) * (fftResult[index] - averageSpectrum[index]);
		}
	}

	for (int index = 0; index < numBands; ++index)
	{
		varianceSpectrum[index] /= (fftHistory.size());
	}
}

void SoundManager_Fmod::initializeBeatDetector()
{
	int bandSize = _samplingFrequency / _windowSize;

	_beatDetector_bandLimits.clear();
	_beatDetector_bandLimits.reserve(4); // bass + lowMidRange * 2

	// BASS 60 hz - 130 hz (Kick Drum)
	_beatDetector_bandLimits.push_back( 60 / bandSize);
	_beatDetector_bandLimits.push_back( 130 / bandSize);

	// LOW MIDRANGE 301 hz - 750 hz (Snare Drum)
	_beatDetector_bandLimits.push_back( 301 / bandSize);
	_beatDetector_bandLimits.push_back( 750 / bandSize);

	_FFTHistory_beatDetector.clear();
}

void SoundManager_Fmod::getBeat(float* spectrum, float* averageSpectrum, bool& isBass, bool& isLowM)
{
	FMOD_DSP_PARAMETER_FFT* dspFFT = NULL;
	FMOD_RESULT result = _dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void **)&dspFFT, 0, 0, 0);

	if (dspFFT)
	{
		// Only read / display half of the buffer typically for analysis 
		// as the 2nd half is usually the same data reversed due to the nature of the way FFT works.
		int length = dspFFT->length / 2;
		int numChannels = dspFFT->numchannels;

		if (length > 0)
		{
			int numBands = _beatDetector_bandLimits.size() / 2;

			for (int numBand = 0; numBand < numBands; ++numBand)
			{
				for (int indexFFT = _beatDetector_bandLimits[numBand]; 
					indexFFT < _beatDetector_bandLimits[numBand + 1];
					++indexFFT)
				{
					for (int channel = 0; channel < numChannels; ++channel)
					{
						spectrum[numBand] += dspFFT->spectrum[channel][indexFFT];
					}
				}
				spectrum[numBand] /= (_beatDetector_bandLimits[numBand + 1] - _beatDetector_bandLimits[numBand]) * numChannels;
			}

			if (_FFTHistory_beatDetector.size() > 0)
			{
				fillAverageSpectrum(averageSpectrum, numBands, _FFTHistory_beatDetector);

				std::vector<float> varianceSpectrum;
				varianceSpectrum.resize(numBands);
				fillVarianceSpectrum(varianceSpectrum.data(), numBands, _FFTHistory_beatDetector, averageSpectrum);
				isBass = (spectrum[0] - 0.05) > beatThreshold(varianceSpectrum[0]) * averageSpectrum[0];
				isLowM = (spectrum[1] - 0.005) > beatThreshold(varianceSpectrum[1]) * averageSpectrum[1];
			}
				
			std::vector<float> fftResult;
			fftResult.reserve(numBands);
			for (int index = 0; index < numBands; ++index)
			{
				fftResult.push_back(spectrum[index]);
			}
			
			if (_FFTHistory_beatDetector.size() >= _FFThistory_MaxSize)
			{
				_FFTHistory_beatDetector.pop_front();
			}

			_FFTHistory_beatDetector.push_back(fftResult);
		}
	}
}

float SoundManager_Fmod::beatThreshold(float variance)
{
	return -15 * variance + 1.55;
}
