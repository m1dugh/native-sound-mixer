#include "./headers/linux-sound-mixer.hpp"
#include "../headers/sound-mixer.hpp"

using namespace LinuxSoundMixer;

namespace SoundMixer
{

	SoundMixer mixer;

	Napi::Object Init(Napi::Env, Napi::Object)
	{
		mixer = SoundMixer();
		exports.Set("GetSessions", Napi::Function::New(env, GetAudioSessionNames));
		exports.Set("GetDevices", Napi::Function::New(env, GetEndpoints));
		exports.Set("GetDefaultDevice", Napi::Function::New(env, GetDefaultEndpoint));

		exports.Set("SetEndpointVolume", Napi::Function::New(env, SetEndpointVolume));
		exports.Set("GetEndpointVolume", Napi::Function::New(env, GetEndpointVolume));
		exports.Set("SetEndpointMute", Napi::Function::New(env, SetEndpointMute));
		exports.Set("GetEndpointMute", Napi::Function::New(env, GetEndpointMute));

		exports.Set("SetAudioSessionVolume", Napi::Function::New(env, SetAudioSessionVolume));
		exports.Set("GetAudioSessionVolume", Napi::Function::New(env, GetAudioSessionVolume));
		exports.Set("SetAudioSessionMute", Napi::Function::New(env, SetAudioSessionMute));
		exports.Set("GetAudioSessionMute", Napi::Function::New(env, GetAudioSessionMute));

		return exports;
	}

	Napi::Array GetAudioSessionNames(const Napi::CallbackInfo &)
	{
	}

	Napi::Array GetEndpoints(Napi::CallbackInfo const &)
	{
	}
	Napi::Object GetDefaultEndpoint(Napi::CallbackInfo const &)
	{
	}

	Napi::Number SetEndpointVolume(Napi::CallbackInfo const &)
	{
	}
	Napi::Number GetEndpointVolume(Napi::CallbackInfo const &)
	{
	}
	Napi::Boolean SetEndpointMute(Napi::CallbackInfo const &)
	{
	}
	Napi::Boolean GetEndpointMute(Napi::CallbackInfo const &)
	{
	}

	Napi::Number SetAudioSessionVolume(Napi::CallbackInfo const &)
	{
	}
	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &)
	{
	}
	Napi::Boolean SetAudioSessionMute(Napi::CallbackInfo const &)
	{
	}
	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &)
	{
	}

};
