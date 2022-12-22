#pragma once

#include <Audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <string>
#include <vector>
#include "sound-mixer-utils.hpp"

#define LEFT 0
#define RIGHT 1

using SoundMixerUtils::DeviceDescriptor;
using SoundMixerUtils::DeviceType;
using SoundMixerUtils::NotificationHandler;
using SoundMixerUtils::VolumeBalance;

namespace WinSoundMixer
{

/**
 *  \typedef void (*on_device_changed_cb_t)
 *  (DeviceDescriptor dev, NotificationHandler)
 *  \brief  The callback triggered by the windows core audio API.
 */
typedef void (*on_device_changed_cb_t)(
    DeviceDescriptor dev, NotificationHandler);

/**
 *  \class AudioSession
 *  \brief A class representing an audio session, that is, a volume output
 *  registered by an application.
 */
class AudioSession {
    public:
        /**
         *  A basic Constructor
         *  \param control  The linked IAudioSessionControl2 struct provided
         *  by the `Windows core audio API`.
         */
        AudioSession(IAudioSessionControl2 *control);

        /**
         *  A basic desctructor.
         */
        ~AudioSession();

        /**
         *  \fn bool GetMute();
         *  \brief returns the mute flag of the audio session.
         *  \returns true if the session is muted.
         *  \sa SetMute
         *  \sa Device::GetMute
         */
        virtual bool GetMute();

        /**
         *  \fn bool SetMute(bool mute);
         *  \brief Sets the mute flag for the audio session.
         *  \param mute the mute flag.
         *  \sa GetMute
         *  \sa Device::SetMute
         */
        virtual void SetMute(bool mute);

        /**
         *  \fn float GetVolume()
         *  \brief Gets the volume scalar value of the audio session,
         *  that is a float from *0* to *1* representing the percentage of
         *  the max volume.
         *  \returns the volume scalar.
         *  \sa AudioSession::SetVolume
         *  \sa Device::SetVolume
         */
        virtual float GetVolume();

        /**
         *  \fn void SetVolume(float volume);
         *  \brief Sets the volume scalar for the audio session.
         *  \param volume The volume scalar to set.
         *  \sa AudioSession::GetVolume
         *  \sa Device::GetVolume
         */
        virtual void SetVolume(float volume);

        /**
         *  \fn void SetVolumeBalance(const VolumeBalance& balance);
         *  \brief Sets the stereo balance for the session if possible.
         *  \see VolumeBalance
         *  \param balance The volume balance to set.
         *  \sa Device::SetVolumeBalance
         */
        virtual void SetVolumeBalance(const VolumeBalance &);

        /**
         *  \fn VolumeBalance GetVolumeBalance()
         *  \brief Gets the volume balance for the audio session if possible.
         *  \remarks if stereo is not available for the session, a 
         *  VolumeBalance with flag `stereo` set to `false` will be returned.
         *  \sa Device::GetVolumeBalance
         *  \returns The volume balance.
         */
        virtual VolumeBalance GetVolumeBalance();

        /**
         *  \fn std::string id()
         *  \brief returns the id string of the audio session.
         *  \remarks depending on the implementation, each audio session id
         *  might not be unique.
         *  \returns The id of the audio session.
         */
        std::string id();

        /**
         *  \fn std::string name()
         *  \brief returns the name of the audio session if applicable.
         *  \remarks depending on the implementation, the name might be an
         *  empty string.
         *  \returns the name of the audio session.
         */
        std::string name();

        /**
         *  \fn std::string path()
         *  \brief Gets the path to the application using the audio session,
         *  that is, the name of the ran process.
         *  \returns The path to the app running.
         */
        std::string path();

        /**
         *  \fn AudioSessionState state()
         *  \brief Gets the state of the audio session.
         *  \returns The audio session state.
         *  \see AudioSessionState
         */
        AudioSessionState state();

    protected:
        IAudioSessionControl2 *control;

        ISimpleAudioVolume *getAudioVolume();
};

class Device {
    public:

        /**
         *  A constructor binding the actual *Windows core audio API* IMMDevice
         *  to the Device object.
         *  \fn Device(IMMDevice *device, on_device_changed_cb_t cb)
         *  \param device The bound IMMDevice
         *  \param cb The callback to run when an event is triggered.
         */
        Device(IMMDevice *, on_device_changed_cb_t cb);

        /**
         *  A simple desctructor.
         *  \remarks Destroying the Device will also destroy the pointed
         *  IMMDevice
         */
        ~Device();

        /**
         *  \fn bool GetMute()
         *  \brief Gets the mute flag for the Device.
         *  \returns The mute flag of the device.
         */
        virtual bool GetMute();

        /**
         *  \fn void SetMute(bool mute);
         *  \brief Sets the mute flag for the Device.
         *  \param mute The mute flag to set to the Device.
         */
        virtual void SetMute(bool mute);

        /**
         *  \fn float GetVolume();
         *  \brief Gets the volume scalar for the device.
         *  \return The volume scalar.
         */
        virtual float GetVolume();

        /**
         *  \fn void SetVolume(float volume);
         *  \brief Sets the volume scalar for the device, that is,
         *  a float from *0* to *1* representing the percentage to the max
         *  volume.
         *  \param volume The volume scalar to set.
         */
        virtual void SetVolume(float volume);

        /**
         *  \fn void SetVolumeBalance(const VolumeBalance &balance);
         *  \brief Sets the volume balance for stereo control for the Device.
         *  \param balance The volume balance to set.
         */
        virtual void SetVolumeBalance(const VolumeBalance &);

        /**
         *  \fn VolumeBalance GetVolumeBalance();
         *  \brief Gets the volume balance for the Device.
         *  \returns The volume balance.
         */
        virtual VolumeBalance GetVolumeBalance();

        /**
         *  \brief      Updates the info of the Device.
         */
        virtual bool Update();

        /**
         *  \brief      Returns whether the device is still available.
         */
        bool IsValid();

        DeviceDescriptor Desc()
        {
            return desc;
        }

        std::vector<AudioSession *> GetAudioSessions();
        AudioSession *GetAudioSessionById(std::string);

        static IMMDeviceEnumerator *GetEnumerator();
        on_device_changed_cb_t _deviceCallback;

        float _oldVolume = 0.F;
        BOOL _oldMute = 0.F;

    protected:
        IMMDevice *device;
        IMMEndpoint *endpoint;
        DeviceDescriptor desc;
        IAudioEndpointVolume *endpointVolume;
        IAudioEndpointVolumeCallback *device_cb;

    private:
        bool valid = true;
};

class SoundMixer {
    public:
        SoundMixer(on_device_changed_cb_t);
        ~SoundMixer();
        std::vector<Device *> GetDevices();
        Device *GetDefaultDevice(DeviceType);

    private:
        IMMDeviceEnumerator *pEnumerator = nullptr;
        std::map<std::string, Device *> devices;

    private:
        /**
         *  \brief  Returns the corresponding device if already in the list,
         *  NULL otherwise.
         *
         *  \fn     Device *getDeviceById(LPWSTR id);
         *  \param id   The id of the device to find.
         *  \returns    The corresponding device if already in the list, NULL
         *  otherwise.
         *  \remarks    The caller is responsible for freeing the provided id.
         */
        Device *getDeviceById(LPWSTR id);

        void filterDevices();
        on_device_changed_cb_t deviceCallback;
};

class SoundMixerAudioEndpointVolumeCallback
: public IAudioEndpointVolumeCallback {
    public:
        SoundMixerAudioEndpointVolumeCallback(Device *dev);

        IFACEMETHODIMP_(ULONG) AddRef();
        IFACEMETHODIMP_(ULONG) Release();

    private:
        IFACEMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
        IFACEMETHODIMP QueryInterface(const IID &iid, void **ppUnk);

    private:
        Device *device;
};
}; // namespace WinSoundMixer
