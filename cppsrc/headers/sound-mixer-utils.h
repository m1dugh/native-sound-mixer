#pragma once

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <vector>

namespace SoundMixerUtils {

	struct DeviceDescriptor
	{
		LPWSTR friendlyName;
		LPWSTR description;
		LPWSTR id;
		EDataFlow dataFlow;
	};

	IAudioSessionControl2* GetAudioSessionByProcessId(IMMDevice*, DWORD);
	IAudioEndpointVolume* GetDeviceEndpointVolume(IMMDevice*);
	ISimpleAudioVolume* GetSessionVolume(IAudioSessionControl2*);
	std::vector<IAudioSessionControl2*> GetAudioSessions(IMMDevice*);
	std::vector<DeviceDescriptor> GetDevices();
	IMMDevice* GetDeviceById(LPWSTR);
};