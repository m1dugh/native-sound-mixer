#include "../headers/sound-mixer.hpp"
#include "./headers/win-sound-mixer.hpp"

#include <iostream>

using namespace WinSoundMixer;
using std::vector;

namespace SoundMixer
{
	WinSoundMixer::SoundMixer *mixer;

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		mixer = new WinSoundMixer::SoundMixer();
		MixerObject::Init(env, exports);
		DeviceObject::Init(env, exports);
		AudioSessionObject::Init(env, exports);

		return exports;
	}

	Napi::Object MixerObject::Init(Napi::Env env, Napi::Object exports)
	{
		Napi::Function sm = DefineClass(env, "SoundMixer", {StaticAccessor<&MixerObject::GetDevices>("devices"), StaticMethod<&MixerObject::GetDefaultDevice>("getDefaultDevice")});

		exports.Set("SoundMixer", sm);

		return exports;
	}

	MixerObject::MixerObject(const Napi::CallbackInfo &info) : Napi::ObjectWrap<MixerObject>(info) {}

	Napi::Value MixerObject::GetDefaultDevice(const Napi::CallbackInfo &info)
	{
		DeviceType type = (DeviceType)info[0].As<Napi::Number>().Int32Value();
		Device *pDevice = mixer->GetDefaultDevice(type);
		return DeviceObject::New(info.Env(), pDevice);
	}

	Napi::Value MixerObject::GetDevices(const Napi::CallbackInfo &info)
	{
		int i = 0;
		Napi::Array result = Napi::Array::New(info.Env());
		for (Device *dev : mixer->GetDevices())
		{
			result.Set(i++, DeviceObject::New(info.Env(), dev));
		}

		return result;
	}

	Napi::Object DeviceObject::Init(Napi::Env env, Napi::Object exports)
	{

		constructor = Napi::Persistent(GetClass(env));

		return exports;
	}

	Napi::Function DeviceObject::GetClass(Napi::Env env)
	{
		Napi::Function func = DefineClass(env, "Device", {InstanceAccessor<&DeviceObject::GetVolume, &DeviceObject::SetVolume>("volume"), InstanceAccessor<&DeviceObject::GetMute, &DeviceObject::SetMute>("mute"), InstanceAccessor<&DeviceObject::GetSessions>("sessions"), InstanceAccessor<&DeviceObject::GetChannelVolume, &DeviceObject::SetChannelVolume>("balance")});

		return func;
	}

	DeviceObject::DeviceObject(const Napi::CallbackInfo &info) : Napi::ObjectWrap<DeviceObject>(info)
	{
	}

	DeviceObject::~DeviceObject()
	{
		delete reinterpret_cast<Device *>(pDevice);
	}

	Napi::Value DeviceObject::New(Napi::Env env, void *device)
	{

		Device *dev = reinterpret_cast<Device *>(device);
		Napi::Object result = constructor.New({});
		Napi::ObjectWrap<DeviceObject>::Unwrap(result)->pDevice = dev;
		result.Set("name", dev->Desc().fullName);
		result.Set("type", (int)dev->Desc().type);

		return result;
	}

	Napi::Value DeviceObject::GetVolume(const Napi::CallbackInfo &info)
	{
		Device *dev = reinterpret_cast<Device *>(pDevice);
		return Napi::Number::New(info.Env(), dev->GetVolume());
	}

	void DeviceObject::SetVolume(const Napi::CallbackInfo &info, const Napi::Value &value)
	{
		float volume = value.As<Napi::Number>().FloatValue();
		Device *dev = reinterpret_cast<Device *>(pDevice);
		dev->SetVolume(volume);
	}

	Napi::Value DeviceObject::GetMute(const Napi::CallbackInfo &info)
	{
		Device *dev = reinterpret_cast<Device *>(pDevice);
		return Napi::Boolean::New(info.Env(), dev->GetMute());
	}

	void DeviceObject::SetMute(const Napi::CallbackInfo &info, const Napi::Value &value)
	{
		bool val = value.As<Napi::Boolean>().Value();
		Device *dev = reinterpret_cast<Device *>(pDevice);
		dev->SetMute(val);
	}

	Napi::Value DeviceObject::GetChannelVolume(const Napi::CallbackInfo &info)
	{
		Device *dev = reinterpret_cast<Device *>(pDevice);
		VolumeBalance balance = dev->GetVolumeBalance();
		Napi::Object result = Napi::Object::New(info.Env());
		result.Set("right", balance.right);
		result.Set("left", balance.left);
		result.Set("stereo", balance.stereo);
		return result;
	}

	void DeviceObject::SetChannelVolume(const Napi::CallbackInfo &info, const Napi::Value &value)
	{
		Napi::Object param = value.As<Napi::Object>();
		if (!param.Has("right") || !param.Has("left"))
		{
			return;
		}
		Device *dev = reinterpret_cast<Device *>(pDevice);
		VolumeBalance balance = {
			param.Get("right").As<Napi::Number>().FloatValue(),
			param.Get("left").As<Napi::Number>().FloatValue(),
			true};

		dev->SetVolumeBalance(balance);
	}

	Napi::Value DeviceObject::GetSessions(const Napi::CallbackInfo &info)
	{
		Device *dev = reinterpret_cast<Device *>(pDevice);
		Napi::Array result = Napi::Array::New(info.Env());
		int i = 0;
		for (AudioSession *s : dev->GetAudioSessions())
		{
			result.Set(i++, AudioSessionObject::New(info.Env(), s));
		}

		return result;
	}

	AudioSessionObject::AudioSessionObject(const Napi::CallbackInfo &info) : Napi::ObjectWrap<AudioSessionObject>(info)
	{
	}

	Napi::Function AudioSessionObject::GetClass(Napi::Env env)
	{
		return DefineClass(env, "AudioSession", {InstanceAccessor<&AudioSessionObject::GetMute, &AudioSessionObject::SetMute>("mute"), InstanceAccessor<&AudioSessionObject::GetVolume, &AudioSessionObject::SetVolume>("volume"), InstanceAccessor<&AudioSessionObject::GetChannelVolume, &AudioSessionObject::SetChannelVolume>("balance")});
	}

	Napi::Object AudioSessionObject::Init(Napi::Env env, Napi::Object exports)
	{
		constructor = Napi::Persistent(GetClass(env));
		return exports;
	}

	AudioSessionObject::~AudioSessionObject()
	{
		delete reinterpret_cast<AudioSession *>(pSession);
	}

	Napi::Value AudioSessionObject::New(Napi::Env env, void *data)
	{
		AudioSession *session = reinterpret_cast<AudioSession *>(data);
		Napi::Object result = Napi::Persistent(GetClass(env)).New({});
		Napi::ObjectWrap<AudioSessionObject>::Unwrap(result)->pSession = session;
		result.Set("name", session->name());
		result.Set("appName", session->path());

		return result;
	}

	Napi::Value AudioSessionObject::GetVolume(const Napi::CallbackInfo &info)
	{
		return Napi::Number::New(info.Env(), reinterpret_cast<AudioSession *>(pSession)->GetVolume());
	}

	void AudioSessionObject::SetVolume(const Napi::CallbackInfo &info, const Napi::Value &value)
	{
		float volume = value.As<Napi::Number>().FloatValue();
		reinterpret_cast<AudioSession *>(pSession)->SetVolume(volume);
	}

	Napi::Value AudioSessionObject::GetMute(const Napi::CallbackInfo &info)
	{
		return Napi::Boolean::New(info.Env(), reinterpret_cast<AudioSession *>(pSession)->GetMute());
	}

	void AudioSessionObject::SetMute(const Napi::CallbackInfo &info, const Napi::Value &value)
	{
		bool val = value.As<Napi::Boolean>().Value();
		reinterpret_cast<AudioSession *>(pSession)->SetMute(val);
	}

	Napi::Value AudioSessionObject::GetChannelVolume(const Napi::CallbackInfo &info)
	{
		AudioSession *session = reinterpret_cast<AudioSession *>(pSession);
		VolumeBalance balance = session->GetVolumeBalance();
		Napi::Object result = Napi::Object::New(info.Env());
		result.Set("right", balance.right);
		result.Set("left", balance.left);
		return result;
	}

	void AudioSessionObject::SetChannelVolume(const Napi::CallbackInfo &info, const Napi::Value &value)
	{
		Napi::Object param = value.As<Napi::Object>();
		if (!param.Has("right") || !param.Has("left"))
		{
			return;
		}
		AudioSession *session = reinterpret_cast<AudioSession *>(pSession);
		VolumeBalance balance = {
			param.Get("right").As<Napi::Number>().FloatValue(),
			param.Get("left").As<Napi::Number>().FloatValue(),
			true};

		session->SetVolumeBalance(balance);
	}

}
