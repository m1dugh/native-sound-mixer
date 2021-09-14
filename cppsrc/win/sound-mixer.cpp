#include "../headers/sound-mixer.hpp"
#include "./headers/win-sound-mixer.hpp"

using namespace WinSoundMixer;
using std::vector;

namespace SoundMixer
{

	WinSoundMixer::SoundMixer *mixer;

	Napi::Object Init(Napi::Env env, Napi::Object exports)
	{
		mixer = new WinSoundMixer::SoundMixer();
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

	Napi::Array GetAudioSessions(Napi::CallbackInfo const &info)
	{

		Napi::Env env = info.Env();

		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected string as device id as only parameter");
		}

		std::string id = info[0].As<Napi::String>().Utf8Value();
		Device *dev = mixer->GetDeviceById(id);
		vector<AudioSession *> sessions = dev->GetAudioSessions();

		Napi::Array sessionNames = Napi::Array::New(env, sessions.size());
		int i = 0;
		for (auto *pSession : sessions)
		{

			Napi::Object val = Napi::Object::New(env);
			val.Set("id", pSession->id());
			val.Set("path", pSession->path());
			val.Set("name", pSession->name());
			val.Set("state", (int)pSession->state());
			sessionNames.Set(i++, val);
		}

		return sessionNames;
	}

	Napi::Array GetDevices(Napi::CallbackInfo const &info)
	{

		Napi::Env env = info.Env();

		vector<Device *> devices = mixer->GetDevices();
		Napi::Array result = Napi::Array::New(env, devices.size());
		int i = 0;
		for (auto *dev : devices)
		{
			Napi::Object obj = Napi::Object::New(env);
			DeviceDescriptor desc = dev->Desc();
			obj.Set("id", desc.id);
			obj.Set("name", desc.fullName);
			obj.Set("type", (int)desc.type);
			result.Set(i++, obj);
		}
		return result;
	}

	Napi::Object GetDefaultDevice(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		Napi::Object obj = Napi::Object::New(env);
		if (info.Length() != 1 || !info[0].IsNumber())
		{
			Napi::Error::New(env, "wrong argument passed, expected number").ThrowAsJavaScriptException();
			return obj;
		}

		int val = info[0].As<Napi::Number>().Int32Value();
		if (val < 0 || val >= EDataFlow::EDataFlow_enum_count)
		{
			Napi::Error::New(env, "illegal argument passed, expected number ranged from 0 to 3").ThrowAsJavaScriptException();
			return obj;
		}

		DeviceType type = (DeviceType)val;
		DeviceDescriptor desc = mixer->GetDefaultDevice(type)->Desc();

		obj.Set("id", desc.id);
		obj.Set("name", desc.fullName);
		obj.Set("type", (int)desc.type);

		return obj;
	}

	void SetDeviceVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 3 || !info[2].IsNumber() || !info[1].IsNumber() || !info[0].IsString())
		{
			throw Napi::TypeError::New(env, "expected 3 numbers as arguments");
		}

		float volume = info[2].As<Napi::Number>().FloatValue();
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < 0.F)
		{
			volume = 0.F;
		}

		std::string id = info[0].As<Napi::String>().Utf8Value();
		Device *dev = mixer->GetDeviceById(id);
		if (dev == nullptr)
		{
			Napi::Error::New(env, "could not find device for the specified id").ThrowAsJavaScriptException();
			return;
		}

		dev->SetVolume(volume);
	}

	Napi::Number GetDeviceVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected string as device id");
		}

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();

		CoInitialize(NULL);

		std::string id = info[0].As<Napi::String>().Utf8Value();
		Device *pDevice = mixer->GetDeviceById(id);
		if (pDevice == nullptr)
		{
			Napi::Error::New(env, "Invalid device id provided").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0);
		}

		return Napi::Number::New(env, pDevice->GetVolume());
	}

	void SetDeviceMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 3 || !info[2].IsBoolean() || !info[0].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected {deviceId: string},{mute: boolean} as arguments");
		}

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		bool mute = info[2].As<Napi::Boolean>().Value();

		std::string id = info[0].As<Napi::String>().Utf8Value();
		Device *dev = mixer->GetDeviceById(id);
		if (dev == nullptr)
		{
			Napi::Error::New(env, "could not find device for the specified id").ThrowAsJavaScriptException();
			return;
		}

		dev->SetMute(mute);
	}

	Napi::Boolean GetDeviceMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 2 || !info[0].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "device id as only argument");
		}
		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string id = info[0].As<Napi::String>().Utf8Value();
		Device *dev = mixer->GetDeviceById(id);
		if (dev == nullptr)
		{
			Napi::Error::New(env, "could not find device for the specified id").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		return Napi::Boolean::New(env, dev->GetMute());
	}

	void SetAudioSessionVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 4 || !info[0].IsString() || !info[2].IsString() || !info[3].IsNumber() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected 3 numbers as arguments");
		}

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();

		float volume = info[3].As<Napi::Number>().FloatValue();
		if (volume > 1.F)
		{
			volume = 1.F;
		}
		else if (volume < 0.F)
		{
			volume = 0.F;
		}

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		Device *pDevice = mixer->GetDeviceById(deviceId);
		if (pDevice == nullptr)
		{
			Napi::Error::New(env, "Invalid device id provided").ThrowAsJavaScriptException();
			return;
		}

		std::string sessionId = info[2].As<Napi::String>().Utf8Value();
		AudioSession *session = pDevice->GetAudioSessionById(sessionId);
		if (session == nullptr)
		{
			Napi::Error::New(env, "Invalid session id provided").ThrowAsJavaScriptException();
			return;
		}

		session->SetVolume(volume);
	}

	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 3 || !info[2].IsString() || !info[0].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected {<deviceId>: string}, {<audio session id>: string} as arguments");
		}

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		Device *pDevice = mixer->GetDeviceById(deviceId);
		if (pDevice == nullptr)
		{
			Napi::Error::New(env, "Invalid device id provided").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0);
		}

		std::string sessionId = info[2].As<Napi::String>().Utf8Value();
		AudioSession *session = pDevice->GetAudioSessionById(sessionId);
		if (session == nullptr)
		{
			Napi::Error::New(env, "Invalid session id provided").ThrowAsJavaScriptException();
			return Napi::Number::New(env, 0);
		}

		return Napi::Number::New(env, session->GetVolume());
	}

	void SetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();
		if (info.Length() != 4 || !info[0].IsString() || !info[2].IsString() || !info[3].IsBoolean() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected 3 numbers as arguments");
		}

		bool mute = info[3].As<Napi::Boolean>().Value();

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();

		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		Device *pDevice = mixer->GetDeviceById(deviceId);
		if (pDevice == nullptr)
		{
			Napi::Error::New(env, "Invalid device id provided").ThrowAsJavaScriptException();
			return;
		}

		std::string sessionId = info[2].As<Napi::String>().Utf8Value();
		AudioSession *session = pDevice->GetAudioSessionById(sessionId);
		if (session == nullptr)
		{
			Napi::Error::New(env, "Invalid session id provided").ThrowAsJavaScriptException();
			return;
		}

		session->SetMute(mute);
	}

	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &info)
	{
		Napi::Env env = info.Env();

		if (info.Length() != 3 || !info[0].IsString() || !info[2].IsString() || !info[1].IsNumber())
		{
			throw Napi::TypeError::New(env, "expected 3 numbers as arguments");
		}

		DeviceType type = (DeviceType)info[1].As<Napi::Number>().Int32Value();
		std::string deviceId = info[0].As<Napi::String>().Utf8Value();
		Device *pDevice = mixer->GetDeviceById(deviceId);
		if (pDevice == nullptr)
		{
			Napi::Error::New(env, "Invalid device id provided").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		std::string sessionId = info[2].As<Napi::String>().Utf8Value();
		AudioSession *session = pDevice->GetAudioSessionById(sessionId);
		if (session == nullptr)
		{
			Napi::Error::New(env, "Invalid session id provided").ThrowAsJavaScriptException();
			return Napi::Boolean::New(env, false);
		}

		return Napi::Boolean::New(env, session->GetMute());
	}

} // namespace SoundMixer
