// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManager.h"
#include "Paths.h"
#include "FileHelper.h"

#include <string>

UAudioManager::UAudioManager()
{
}

UAudioManager::~UAudioManager()
{
}

int32 UAudioManager::InitializeManager()
{
	_soundManager = std::unique_ptr<SoundManager_Fmod>(new SoundManager_Fmod());
	int result = _soundManager->initialize();
	if (result > 0)
	{
		return result;
	}
	return 0;
}

int32 UAudioManager::PlaySong(int numSong)
{
	FString songsPath = FPaths::ProjectContentDir() + "NonAssets/";

	switch (numSong)
	{
	case 0: currentSongName = "A Drop A Day - Fairy Dust"; break;
	case 1: currentSongName = "Eiffel 65 - Daba Dee"; break;
	case 2: currentSongName = "Jason Shaw  - Big car theft"; break;
	case 3: currentSongName = "Sample_HatTrain"; break;
	case 4: currentSongName = "Sample_KickSnareHat"; break;
	}

	FString songFile(songsPath + currentSongName + ".wav");

	uint8* data;
	unsigned int dataLength = 0;

	TArray <uint8> rawFile;
	FFileHelper::LoadFileToArray(rawFile, *songFile);

	data = rawFile.GetData();
	dataLength = rawFile.Num() * sizeof(uint8);

	int32 result = _soundManager->loadSoundFromMemory(reinterpret_cast<char*>(data), dataLength);
	if (result > 0)
	{
			return result; //missing file
	}

	_soundManager->playSound();
	return 0;
}

void UAudioManager::PauseSong(bool unPause)
{
	_soundManager->pauseSound(unPause);
}

const FString& UAudioManager::GetSongName() const
{
	return currentSongName;
}

void UAudioManager::Update()
{
	_soundManager->update();
}

int32 UAudioManager::InitSpectrum_Linear(const int32 maxBars)
{
	return _soundManager->initializeSpectrum_Linear(maxBars);
}

void UAudioManager::GetSpectrum_Linear(TArray<float>& frequencyValues, TArray<float>& frequencyAverageValues, int32 numBars)
{
	frequencyValues.Init(0.0, numBars);
	frequencyAverageValues.Init(0.0, numBars);
	_soundManager->getSpectrum_Linear(frequencyValues.GetData(), frequencyAverageValues.GetData());
}

int32 UAudioManager::InitSpectrum_Log(const int32 maxBars)
{
	return _soundManager->initializeSpectrum_Log(maxBars);
}

void UAudioManager::GetSpectrum_Log(TArray<float>& frequencyValues, TArray<float>& frequencyAverageValues, int32 numBars)
{
	frequencyValues.Init(0.0, numBars);
	frequencyAverageValues.Init(0.0, numBars);
	_soundManager->getSpectrum_Log(frequencyValues.GetData(), frequencyAverageValues.GetData());
}

void UAudioManager::InitBeatDetector()
{
	return _soundManager->initializeBeatDetector();
}

void UAudioManager::GetBeat(TArray<float>& frequencyValues, TArray<float>& frequencyAverageValues, bool& isBass, bool& isLowM)
{
	frequencyValues.Init(0.0, 2);
	frequencyAverageValues.Init(0.0, 2);
	_soundManager->getBeat(frequencyValues.GetData(), frequencyAverageValues.GetData(), isBass, isLowM);
}
