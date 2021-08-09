#pragma once
#include <iostream>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <vector>
#include <exception>

namespace SoundMixerUtils
{

	class SoundMixerException : public std::exception
	{
	public:
		SoundMixerException(std::string const &message = "an error occured in sound mixer process") throw() : m_message(message) {}

		virtual const char *what() const throw()
		{
			return m_message.c_str();
		}

	private:
		std::string m_message;
	};

	struct DeviceDescriptor
	{
		std::string fullName;
		std::string id;
		EDataFlow dataFlow;
	};

	IAudioSessionControl2 *GetAudioSessionByGUID(IMMDevice *, LPWSTR);
	IAudioEndpointVolume *GetDeviceEndpointVolume(IMMDevice *);
	ISimpleAudioVolume *GetSessionVolume(IAudioSessionControl2 *);
	std::vector<IAudioSessionControl2 *> GetAudioSessions(IMMDevice *);
	std::vector<DeviceDescriptor> GetDevices();
	DeviceDescriptor GetDevice(EDataFlow);
	IMMDevice *GetDeviceById(LPWSTR);
};