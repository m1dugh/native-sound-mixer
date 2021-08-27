#pragma once

#include "./sound-mixer-utils.hpp"

#include <napi.h>

namespace SoundMixer
{

	Napi::Object Init(Napi::Env, Napi::Object);

	Napi::Array GetAudioSessionNames(Napi::CallbackInfo const &);
	Napi::Array GetEndpoints(Napi::CallbackInfo const &);
	Napi::Object GetDefaultEndpoint(Napi::CallbackInfo const &);

	Napi::Number SetEndpointVolume(Napi::CallbackInfo const &);
	Napi::Number GetEndpointVolume(Napi::CallbackInfo const &);
	Napi::Boolean SetEndpointMute(Napi::CallbackInfo const &);
	Napi::Boolean GetEndpointMute(Napi::CallbackInfo const &);

	Napi::Number SetAudioSessionVolume(Napi::CallbackInfo const &);
	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &);
	Napi::Boolean SetAudioSessionMute(Napi::CallbackInfo const &);
	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &);
} // namespace SoundMixer