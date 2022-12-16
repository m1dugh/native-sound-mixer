#pragma once

#include "sound-mixer-utils.hpp"
#include <napi.h>
#include <map>
#include <vector>

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
        public:
        static Napi::Object Init(Napi::Env, Napi::Object);
        DeviceObject(const Napi::CallbackInfo &info);
        virtual ~DeviceObject();
        static Napi::Value New(Napi::Env, void *);

        Napi::Value GetName(const Napi::CallbackInfo &info);
        Napi::Value GetType(const Napi::CallbackInfo &info);

        Napi::Value GetVolume(const Napi::CallbackInfo &info);
        Napi::Value GetMute(const Napi::CallbackInfo &info);
        void SetVolume(const Napi::CallbackInfo &info, const Napi::Value &value);
        void SetMute(const Napi::CallbackInfo &info, const Napi::Value &value);

        Napi::Value RegisterEvent(const Napi::CallbackInfo &info);
        Napi::Value RemoveEvent(const Napi::CallbackInfo &info);

        void SetChannelVolume(const Napi::CallbackInfo &info, const Napi::Value &value);
        Napi::Value GetChannelVolume(const Napi::CallbackInfo &info);

        Napi::Value GetSessions(const Napi::CallbackInfo &info);

        bool Update();


        public:
        static Napi::FunctionReference *constructor;

        private:
        Napi::Value GetName();
        static Napi::Function GetClass(Napi::Env);

        private:
        void *pDevice;
        SoundMixerUtils::DeviceDescriptor Desc();
    };

    class MixerObject : public Napi::ObjectWrap<MixerObject>
    {
        public:
            static Napi::Object Init(Napi::Env, Napi::Object);
            static Napi::Value GetDevices(const Napi::CallbackInfo &info);
            MixerObject(const Napi::CallbackInfo &info);
            virtual ~MixerObject();
            static Napi::Value GetDefaultDevice(const Napi::CallbackInfo &info);
            
 /*           static void on_device_change_cb(SoundMixerUtils::DeviceDescriptor d, SoundMixerUtils::NotificationHandler data);

        public:
            static SoundMixerUtils::EventPool *eventPool;*/
    };
} // namespace SoundMixer
