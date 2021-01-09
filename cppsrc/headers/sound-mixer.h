#include <napi.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <vector>

namespace SoundMixer
{
	Napi::Object Init(Napi::Env, Napi::Object);

	Napi::Array GetAudioSessionNames(Napi::CallbackInfo const &);

	void SetEndpointVolume(Napi::CallbackInfo const &);
	Napi::Number GetEndpointVolume(Napi::CallbackInfo const &);
	void SetEndpointMute(Napi::CallbackInfo const &);
	Napi::Boolean GetEndpointMute(Napi::CallbackInfo const &);

	void SetAudioSessionVolume(Napi::CallbackInfo const &);
	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &);
	void SetAudioSessionMute(Napi::CallbackInfo const &);
	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &);

	IMMDevice *GetDevice(EDataFlow, ERole);
	IAudioSessionControl2 *GetAudioSessionByProcessId(IMMDevice *, std::string);
	IAudioEndpointVolume *GetDeviceEndpointVolume(IMMDevice *);
	ISimpleAudioVolume *GetSessionVolume(IAudioSessionControl2 *);
	std::vector<IAudioSessionControl2*> GetAudioSessions(IMMDevice*);
} // namespace SoundMixer