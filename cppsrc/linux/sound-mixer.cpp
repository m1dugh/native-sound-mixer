#include "./headers/linux-sound-mixer.hpp"
#include "../headers/sound-mixer.hpp"

using namespace LinuxSoundMixer;

namespace SoundMixer
{

	LinuxSoundMixer::SoundMixer mixer;

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		mixer = LinuxSoundMixer::SoundMixer();
		exports.Set("GetSessions", Napi::Function::New(env, GetAudioSessions));
		exports.Set("GetDevices", Napi::Function::New(env, GetDevices));
		exports.Set("GetDefaultDevice", Napi::Function::New(env, GetDefaultDevice));

		exports.Set("SetDeviceVolume", Napi::Function::New(env, SetDeviceVolume));
		exports.Set("GetDeviceVolume", Napi::Function::New(env, GetDeviceVolume));
		exports.Set("SetDeviceMute", Napi::Function::New(env, SetDeviceMute));
		exports.Set("GetDeviceMute", Napi::Function::New(env, GetDeviceMute));

		exports.Set("SetAudioSessionVolume", Napi::Function::New(env, SetAudioSessionVolume));
		exports.Set("GetAudioSessionVolume", Napi::Function::New(env, GetAudioSessionVolume));
		exports.Set("SetAudioSessionMute", Napi::Function::New(env, SetAudioSessionMute));
		exports.Set("GetAudioSessionMute", Napi::Function::New(env, GetAudioSessionMute));

		return exports;
	}

	Napi::Array GetAudioSessions(const Napi::CallbackInfo &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 1 || !info[0].IsString())
		{
			throw Napi::TypeError::New(env, "expected string as device id as only parameter");
		}

		std::string deviceName = info[0].As<Napi::String>().Utf8Value();

		mixer.GetDeviceByName(deviceName, DeviceType::OUTPUT);
	}

	Napi::Array GetDevices(Napi::CallbackInfo const &)
	{
	}
	Napi::Object GetDefaultDevice(Napi::CallbackInfo const &)
	{
	}

	void SetDeviceVolume(Napi::CallbackInfo const &)
	{
	}
	Napi::Number GetDeviceVolume(Napi::CallbackInfo const &)
	{
	}
	void SetDeviceMute(Napi::CallbackInfo const &)
	{
	}
	Napi::Boolean GetDeviceMute(Napi::CallbackInfo const &)
	{
	}

	void SetAudioSessionVolume(Napi::CallbackInfo const &)
	{
	}
	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &)
	{
	}
	void SetAudioSessionMute(Napi::CallbackInfo const &)
	{
	}
	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &)
	{
	}

};
