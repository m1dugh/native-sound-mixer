#pragma once

#include "./sound-mixer-utils.hpp"

#include <napi.h>

namespace SoundMixer
{
    Napi::Object Init(Napi::Env, Napi::Object);

    class AudioSessionObject : public Napi::ObjectWrap<AudioSessionObject>
    {
        public:
        static Napi::Object Init(Napi::Env, Napi::Object);
        static Napi::Value New(Napi::Env, void *);
        AudioSessionObject(const Napi::CallbackInfo &info);
        virtual ~AudioSessionObject();

        Napi::Value GetVolume(const Napi::CallbackInfo &info);
        Napi::Value GetMute(const Napi::CallbackInfo &info);
        void SetVolume(const Napi::CallbackInfo &info, const Napi::Value &value);
        void SetMute(const Napi::CallbackInfo &info, const Napi::Value &value);

        void SetChannelVolume(const Napi::CallbackInfo &info, const Napi::Value &);
        Napi::Value GetChannelVolume(const Napi::CallbackInfo &info);

        private:
        static Napi::Function GetClass(Napi::Env);

        public:
        void *pSession;
        static Napi::FunctionReference *constructor;
    };

    class DeviceObject : public Napi::ObjectWrap<DeviceObject>
    {
        static Napi::FunctionReference *constructor;
        public:
        static Napi::Object Init(Napi::Env, Napi::Object);
        DeviceObject(const Napi::CallbackInfo &info);
        virtual ~DeviceObject();
        static Napi::Value New(Napi::Env, void *);

        Napi::Value GetVolume(const Napi::CallbackInfo &info);
        Napi::Value GetMute(const Napi::CallbackInfo &info);
        void SetVolume(const Napi::CallbackInfo &info, const Napi::Value &value);
        void SetMute(const Napi::CallbackInfo &info, const Napi::Value &value);

        void SetChannelVolume(const Napi::CallbackInfo &info, const Napi::Value &value);
        Napi::Value GetChannelVolume(const Napi::CallbackInfo &info);

        Napi::Value GetSessions(const Napi::CallbackInfo &info);

        private:
        Napi::Value GetName();
        static Napi::Function GetClass(Napi::Env);

        private:
        SoundMixerUtils::DeviceDescriptor desc;

        void *pDevice;
    };

    class MixerObject : public Napi::ObjectWrap<MixerObject>
    {
        public:
            static Napi::Object Init(Napi::Env, Napi::Object);
            static Napi::Value GetDevices(const Napi::CallbackInfo &info);
            MixerObject(const Napi::CallbackInfo &info);
            static Napi::Value GetDefaultDevice(const Napi::CallbackInfo &info);
    };

} // namespace SoundMixer
