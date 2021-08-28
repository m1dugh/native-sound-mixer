#include "./headers/linux-sound-mixer.hpp"
#include "../headers/sound-mixer.hpp"


using namespace LinuxSoundMixer;

namespace SoundMixer
{

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
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
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();

		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected string as device id as only parameter");
		}

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();

		std::string deviceName = info[0].As<Napi::String>().Utf8Value();

		Napi::Array result = Napi::Array::New(env);

		_Device *dev = mixer.GetDeviceByName(deviceName, type);
		if (dev == nullptr)
		{
			return result;
		}

		int i = 0;
		for (auto *session : dev->GetAudioSessions())
		{
			Napi::Object obj = Napi::Object::New(env);
			obj.Set("id", session->appName());
			obj.Set("path", session->description());
			obj.Set("state", 1);
			result.Set(i++, obj);
		}
		return result;
	}

	Napi::Array GetDevices(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		Napi::Array result = Napi::Array::New(env);

		int i = 0;
		for (auto *d : mixer.GetDevices())
		{
			Napi::Object obj = Napi::Object::New(env);
			obj.Set("id", d->name());
			obj.Set("name", d->friendlyName());
			obj.Set("type", (int)d->type());
			result.Set(i++, obj);
		}

		return result;
	}
	Napi::Object GetDefaultDevice(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();

		Napi::Env env = info.Env();
		if (info.Length() != 1 || !info[0].IsNumber())
		{
			Napi::Error::New(env, "bad arguments given, expected DeviceType as only argument").ThrowAsJavaScriptException();
			return Napi::Object::New(env);
		}

		DeviceType type = (DeviceType)info[0].As<Napi::Number>().Int32Value();
		for (auto *d : mixer.GetDevices())
		{
			if (d->type() == type && d->index == 1)
			{
				DeviceDescriptor desc = d->ToDeviceDescriptor();
				Napi::Object obj = Napi::Object::New(env);
				obj.Set("id", desc.id);
				obj.Set("name", desc.fullName);
				obj.Set("type", (int)desc.type);
				return obj;
			}
		}

		return Napi::Object::New(env);
	}

	void SetDeviceVolume(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsNumber())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>, <new volume>").ThrowAsJavaScriptException();
			return;
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		float volume = info[2].As<Napi::Number>().FloatValue();
		if (volume < 0.F)
		{
			volume = 0.F;
		}
		else if (volume > 1.F)
		{
			volume = 1.F;
		}

		mixer.GetDeviceByName(deviceId, type)->SetVolume(volume);
	}
	Napi::Number GetDeviceVolume(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>").ThrowAsJavaScriptException();
			return Napi::Number::New(env, .0F);
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();

		return Napi::Number::New(env, mixer.GetDeviceByName(deviceId, type)->GetVolume());
	}
	void SetDeviceMute(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsBoolean())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>, <new mute flag>").ThrowAsJavaScriptException();
			return;
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		bool muteFlag = info[2].As<Napi::Boolean>().Value();

		mixer.GetDeviceByName(deviceId, type)->SetMute(muteFlag);
	}
	Napi::Boolean GetDeviceMute(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();

		return Napi::Boolean::New(env, mixer.GetDeviceByName(deviceId, type)->GetMute());
	}

	void SetAudioSessionVolume(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 4 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString() || !info[3].IsNumber())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>, <session id>, <new volume>").ThrowAsJavaScriptException();
			return;
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string sessionId = info[2].As<Napi::String>().Utf8Value();
		float volume = info[3].As<Napi::Number>().FloatValue();
		if (volume < 0.F)
		{
			volume = 0.F;
		}
		else if (volume > 1.F)
		{
			volume = 1.F;
		}

		_Device *d = mixer.GetDeviceByName(deviceId, type);
		for (auto *s : d->GetAudioSessions())
		{
			if (s->appName() == sessionId)
			{
				s->SetVolume(volume);
				return;
			}
		}
	}
	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>, <session id>").ThrowAsJavaScriptException();
			return Napi::Number::New(env, .0F);
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string sessionId = info[2].As<Napi::String>().Utf8Value();

		_Device *d = mixer.GetDeviceByName(deviceId, type);
		for (auto *s : d->GetAudioSessions())
		{
			if (s->appName() == sessionId)
			{
				return Napi::Number::New(env, s->GetVolume());
			}
		}

		return Napi::Number::New(env, .0F);
	}
	void SetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 4 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString() || !info[3].IsBoolean())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>, <session id>, <new mute flag>").ThrowAsJavaScriptException();
			return;
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string sessionId = info[2].As<Napi::String>().Utf8Value();
		bool muteFlag = info[3].As<Napi::Boolean>().Value();

		_Device *d = mixer.GetDeviceByName(deviceId, type);
		for (auto *s : d->GetAudioSessions())
		{
			if (s->appName() == sessionId)
			{
				s->SetMute(muteFlag);
				return;
			}
		}
	}
	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		LinuxSoundMixer::SoundMixer mixer = LinuxSoundMixer::SoundMixer();
		Napi::Env env = info.Env();
		if (info.Length() != 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString())
		{
			Napi::Error::New(env, "wrong arguments passed expected <device id>, <device type>, <session id>").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string sessionId = info[2].As<Napi::String>().Utf8Value();

		_Device *d = mixer.GetDeviceByName(deviceId, type);
		for (auto *s : d->GetAudioSessions())
		{
			if (s->appName() == sessionId)
			{
				return Napi::Boolean::New(env, s->GetMute());
			}
		}

		return Napi::Boolean::New(env, false);
	}

};
