#pragma once

#include "../../headers/sound-mixer-utils.hpp"
#include <string>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <vector>

using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;

namespace WinSoundMixer
{

	class AudioSessionEvents : public IAudioSessionEvents
	{
	};

	class AudioSession
	{
	public:
		AudioSession(IAudioSessionControl2 *control);
		~AudioSession();

		virtual bool GetMute();
		virtual void SetMute(bool);
		virtual float GetVolume();
		virtual void SetVolume(float);

		std::string id();
		std::string name();
		std::string path();
		AudioSessionState state();

	protected:
		IAudioSessionControl2 *control;

		ISimpleAudioVolume *getAudioVolume();
	};

	class Device
	{
	public:
		Device(IMMDevice *);
		~Device();

		virtual bool GetMute();
		virtual void SetMute(bool);
		virtual float GetVolume();
		virtual void SetVolume(float);

		virtual void Reload();

		DeviceDescriptor Desc()
		{
			return desc;
		}

		std::vector<AudioSession *> GetAudioSessions();
		AudioSession *GetAudioSessionById(std::string);

	protected:
		IMMDevice *device;
		IMMEndpoint *endpoint;
		std::vector<AudioSession *> sessions;
		DeviceDescriptor desc;

		IAudioEndpointVolume *getAudioEndpointVolume();
	};

	class SoundMixer
	{
	public:
		SoundMixer();
		~SoundMixer();
		std::vector<Device *> GetDevices();
		Device *GetDeviceById(std::string);
		Device *GetDefaultDevice(DeviceType);
		void Reload();

	private:
		std::vector<Device *> devices;
		IMMDeviceEnumerator *pEnumerator = nullptr;
		Device *defaultOutputDevice = nullptr, *defaultInputDevice = nullptr;
	};
};