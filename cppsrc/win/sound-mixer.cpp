#include "../headers/sound-mixer.hpp"
#include "./headers/win-sound-mixer.hpp"

#include <iostream>

using namespace SoundMixerUtils;
using namespace WinSoundMixer;
using std::vector;

namespace SoundMixer
{
Napi::FunctionReference *AudioSessionObject::constructor;
Napi::FunctionReference *DeviceObject::constructor;
EventPool *MixerObject::eventPool;

	WinSoundMixer::SoundMixer *mixer;

    void on_device_changed_cb(DeviceDescriptor desc, int flags, PAUDIO_VOLUME_NOTIFICATION_DATA data) {
        if(flags & DEVICE_CHANGE_MASK_MUTE) {
            vector<Napi::FunctionReference *> listeners =
                MixerObject::eventPool->GetListeners(desc, EventType::MUTE);

            for (Napi::FunctionReference *cb : listeners) {
                // TODO: execute callback
                // cb->Call({ NULL });
            }
        }

        if(flags & DEVICE_CHANGE_MASK_VOLUME) {
            vector<Napi::FunctionReference *> listeners =
                MixerObject::eventPool->GetListeners(desc, EventType::VOLUME);
            for(Napi::FunctionReference *cb : listeners) {
                // TODO: execute callback
                // cb->Call({NULL});
            }
        }
    }

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		mixer = new WinSoundMixer::SoundMixer(NULL);
		MixerObject::Init(env, exports);
		DeviceObject::Init(env, exports);
		AudioSessionObject::Init(env, exports);

		return exports;
	}

	Napi::Object MixerObject::Init(Napi::Env env, Napi::Object exports)
	{
		Napi::Function sm = DefineClass(env, "SoundMixer", {StaticAccessor<&MixerObject::GetDevices>("devices"), StaticMethod<&MixerObject::GetDefaultDevice>("getDefaultDevice")});


		exports.Set("SoundMixer", sm);
        eventPool = new EventPool();
		return exports;
	}

	MixerObject::MixerObject(const Napi::CallbackInfo &info) : Napi::ObjectWrap<MixerObject>(info) {}

    MixerObject::~MixerObject() {
        delete eventPool;
        delete mixer;
        delete AudioSessionObject::constructor;
        delete DeviceObject::constructor;
    }

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
        constructor = new Napi::FunctionReference();
        Napi::Function f = GetClass(env);
        exports.Set("Device", f);
		*constructor = Napi::Persistent(f);
		return exports;
	}

	Napi::Function DeviceObject::GetClass(Napi::Env env)
	{
		Napi::Function func = DefineClass(env, "Device", {
            InstanceAccessor<&DeviceObject::GetVolume, &DeviceObject::SetVolume>("volume"),
            InstanceAccessor<&DeviceObject::GetMute, &DeviceObject::SetMute>("mute"),
            InstanceAccessor<&DeviceObject::GetSessions>("sessions"),
            InstanceAccessor<&DeviceObject::GetChannelVolume, &DeviceObject::SetChannelVolume>("balance"),
            InstanceMethod<&DeviceObject::RegisterEvent>("on"),
            InstanceMethod<&DeviceObject::RemoveEvent>("removeListener")
        });

		return func;
	}

	DeviceObject::DeviceObject(const Napi::CallbackInfo &info) : Napi::ObjectWrap<DeviceObject>(info)
	{
	}

	DeviceObject::~DeviceObject() {}

	Napi::Value DeviceObject::New(Napi::Env env, void *device)
	{

		Device *dev = reinterpret_cast<Device *>(device);
		Napi::Object result = constructor->New({});
        Napi::ObjectWrap<DeviceObject>::Unwrap(result)->desc = dev->Desc();
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

    Napi::Value DeviceObject::RegisterEvent(const Napi::CallbackInfo &info) {
        Napi::Env env = info.Env();
        if(info.Length() != 2 || !info[0].IsString() || !info[1].IsFunction()) {
            Napi::Error::New(env, "Expected <event-type> <function>").ThrowAsJavaScriptException();
            return Napi::Number::New(env, -1);
        }

        std::string eventName = info[0].As<Napi::String>().Utf8Value();
        EventType eventType;
        if(eventName == "volume")
            eventType = EventType::VOLUME;
        else if(eventName == "mute")
            eventType = EventType::MUTE;
        else
            return Napi::Number::New(env, -1);

        Napi::Function func = info[1].As<Napi::Function>();
        Napi::FunctionReference *ref = new Napi::FunctionReference();
        *ref = Napi::Persistent(func);

        int handler = MixerObject::eventPool->RegisterEvent(desc, eventType, ref);
        return Napi::Number::New(env, handler);
    }

    Napi::Value DeviceObject::RemoveEvent(const Napi::CallbackInfo &info) {
        Napi::Env env = info.Env();
        // expects EventType and event id
        if(info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber()) {
            Napi::Error::New(env, "Expected <event-type> <callback-handler>").ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }
        std::string eventName = info[0].As<Napi::String>().Utf8Value();
        EventType eventType;
        if(eventName == "volume")
            eventType = EventType::VOLUME;
        else if(eventName == "mute")
            eventType = EventType::MUTE;
        else
            return Napi::Number::New(env, -1);
        int handler = info[1].As<Napi::Number>().Int32Value();
        bool res = MixerObject::eventPool->RemoveEvent(desc, eventType, handler);

        return Napi::Boolean::New(env, res);
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
        constructor = new Napi::FunctionReference();
		*constructor = Napi::Persistent(GetClass(env));
		return exports;
	}

	AudioSessionObject::~AudioSessionObject()
	{
		delete reinterpret_cast<AudioSession *>(pSession);
	}

	Napi::Value AudioSessionObject::New(Napi::Env env, void *data)
	{
		AudioSession *session = reinterpret_cast<AudioSession *>(data);
		Napi::Object result = constructor->New({});
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
