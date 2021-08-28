#pragma once

#include "../../headers/sound-mixer-utils.hpp"
#include <iostream>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <vector>

using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;

namespace WinSoundMixer
{

	IAudioSessionControl2 *GetAudioSessionByGUID(IMMDevice *, LPWSTR);
	IAudioEndpointVolume *GetDeviceEndpointVolume(IMMDevice *);
	ISimpleAudioVolume *GetSessionVolume(IAudioSessionControl2 *);
	std::vector<IAudioSessionControl2 *> GetAudioSessions(IMMDevice *);
	std::vector<DeviceDescriptor> GetEndpoints();
	DeviceDescriptor GetDevice(EDataFlow);
	IMMDevice *GetDeviceById(LPWSTR);
};