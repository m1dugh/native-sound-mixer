#pragma once

#include "../../headers/sound-mixer-utils.hpp"
#include <string>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <vector>

#define LEFT 0
#define RIGHT 1

using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;
using SoundMixerUtils::VolumeBalance;

namespace WinSoundMixer
{

	class AudioSession
	{
	public:
		AudioSession(IAudioSessionControl2 *control);
		~AudioSession();

		virtual bool GetMute();
		virtual void SetMute(bool);
		virtual float GetVolume();
		virtual void SetVolume(float);

		virtual void SetVolumeBalance(const VolumeBalance&);
		virtual VolumeBalance GetVolumeBalance();

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

		virtual void SetVolumeBalance(const VolumeBalance&);
		virtual VolumeBalance GetVolumeBalance();

        /**
         *  \brief      Updates the info of the Device.
         */
        virtual bool Update();

        /**
         *  \brief      Returns whether the device is still available.
         */
        bool IsValid();

		DeviceDescriptor Desc()
		{
			return desc;
		}

		std::vector<AudioSession *> GetAudioSessions();
		AudioSession *GetAudioSessionById(std::string);

        static IMMDeviceEnumerator *GetEnumerator();

	protected:
		IMMDevice *device;
		IMMEndpoint *endpoint;
		DeviceDescriptor desc;
        IAudioEndpointVolume *endpointVolume;

    private:
        bool valid = true;

	};

	class SoundMixer
	{
	public:
		SoundMixer();
		~SoundMixer();
		std::vector<Device *> GetDevices();
		Device *GetDefaultDevice(DeviceType);

	private:
		IMMDeviceEnumerator *pEnumerator = nullptr;
        std::map<std::string, Device *> devices;

    private:
        void filterDevices();
	};

    class SoundMixerAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback {
    public:
        SoundMixerAudioEndpointVolumeCallback();

        IFACEMETHODIMP_(ULONG) AddRef();
        IFACEMETHODIMP_(ULONG) Release();
    private:
        IFACEMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
        IFACEMETHODIMP QueryInterface(const IID& iid, void **ppUnk);
    };
};
