#pragma once

#include "../../headers/sound-mixer-utils.hpp"
#include <pulse/pulseaudio.h>
#include <vector>
#include <string>

using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;
using SoundMixerUtils::VolumeBalance;

namespace LinuxSoundMixer
{

	typedef struct _PAControls
	{
		pa_mainloop *mainloop;
		pa_mainloop_api *api;
		pa_context *ctx;
	} _PAControls;

	class _AudioSession
	{
	public:
		uint32_t index;
		virtual std::string description() = 0;
		virtual std::string appName() = 0;

	protected:
		_PAControls pa;

	public:
		_AudioSession(_PAControls, uint32_t);
		virtual ~_AudioSession();
		virtual float GetVolume() = 0;
		virtual void SetVolume(float) = 0;
		virtual bool GetMute() = 0;
		virtual void SetMute(bool) = 0;
	};

	class InputAudioSession : public _AudioSession
	{
	public:
		InputAudioSession(_PAControls, uint32_t);
		float GetVolume();
		void SetVolume(float);
		bool GetMute();
		void SetMute(bool);

	public:
		std::string description();
		std::string appName();

	private:
		pa_source_output_info *GetInfo();
		pa_proplist *GetProps();
	};

	class OutputAudioSession : public _AudioSession
	{
	public:
		OutputAudioSession(_PAControls, uint32_t);
		float GetVolume();
		void SetVolume(float);
		bool GetMute();
		void SetMute(bool);

	public:
		std::string description();
		std::string appName();

	private:
		pa_sink_input_info *GetInfo();
		pa_proplist *GetProps();
	};

	class _Device
	{
	protected:
		_PAControls pa;

	public:
		uint32_t index;
		virtual std::string name() = 0;
		virtual std::string friendlyName() = 0;
		virtual DeviceType type() = 0;
		virtual DeviceDescriptor ToDeviceDescriptor();

	public:
		_Device(_PAControls, uint32_t index);
		virtual ~_Device();
		virtual float GetVolume() = 0;
		virtual void SetVolume(float) = 0;
		virtual bool GetMute() = 0;
		virtual void SetMute(bool) = 0;
		virtual void SetVolumeBalance(const VolumeBalance &) = 0;
		virtual VolumeBalance GetVolumeBalance() = 0;
		virtual std::vector<_AudioSession *> GetAudioSessions() = 0;
	};

	class OutputDevice : public _Device
	{
	public:
		OutputDevice(_PAControls controls, uint32_t index);
		std::vector<_AudioSession *> GetAudioSessions();
		float GetVolume();
		void SetVolume(float);
		bool GetMute();
		void SetMute(bool);
		void SetVolumeBalance(const VolumeBalance &);
		VolumeBalance GetVolumeBalance();

	public:
		std::string friendlyName();
		std::string name();
		DeviceType type();

	private:
		pa_sink_info *GetInfo();
		pa_proplist *GetProps();
	};

	class InputDevice : public _Device
	{
	private:
		pa_source_info *GetInfo();
		pa_proplist *GetProps();

	public:
		InputDevice(_PAControls controls, uint32_t index) : _Device(controls, index){};
		std::vector<_AudioSession *> GetAudioSessions();
		float GetVolume();
		void SetVolume(float);
		bool GetMute();
		void SetMute(bool);
		void SetVolumeBalance(const VolumeBalance &);
		VolumeBalance GetVolumeBalance();

	public:
		std::string friendlyName();
		std::string name();
		DeviceType type();
	};

	class SoundMixer
	{
	public:
		SoundMixer();
		virtual ~SoundMixer();
		std::vector<_Device *> GetDevices();
		_Device *GetDefaultDevice(DeviceType);
		_Device *GetDeviceByName(std::string name, DeviceType type);

	private:
		_PAControls pa;
		int ready = 0;
	};

};
