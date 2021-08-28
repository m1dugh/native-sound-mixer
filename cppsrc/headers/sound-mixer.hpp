#pragma once

#include "./sound-mixer-utils.hpp"

#include <napi.h>

namespace SoundMixer
{

	Napi::Object Init(Napi::Env, Napi::Object);

	/**
	 * @param string device name
	 * @param DeviceType device type
	 * @returns audio sessions linked to device
	 */
	Napi::Array GetAudioSessions(Napi::CallbackInfo const &);

	/**
	 * @returns all the devices
	 */
	Napi::Array GetDevices(Napi::CallbackInfo const &);

	/**
	 * @param DeviceType the device type
	 * @returns the default device
	 */
	Napi::Object GetDefaultDevice(Napi::CallbackInfo const &);

	/**
	 * @param string device id
	 * @param DeviceType the device type
	 * @param float the volume to set
	 * @returns the new volume
	 */
	Napi::Number SetDeviceVolume(Napi::CallbackInfo const &);

	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @returns the volume
	 */
	Napi::Number GetDeviceVolume(Napi::CallbackInfo const &);
	
	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @param bool the mute flag
	 * @returns the new mute flag
	 */
	Napi::Boolean SetDeviceMute(Napi::CallbackInfo const &);
	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @returns the mute flag
	 */
	Napi::Boolean GetDeviceMute(Napi::CallbackInfo const &);

	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @param string the session id
	 * @param float the volume to set
	 * @returns the new volume
	 */
	Napi::Number SetAudioSessionVolume(Napi::CallbackInfo const &);
	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @param string the session id
	 * @returns the new volume
	 */
	Napi::Number GetAudioSessionVolume(Napi::CallbackInfo const &);
	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @param string the session id
	 * @param bool the mute flag
	 * @returns the new mute flag
	 */
	Napi::Boolean SetAudioSessionMute(Napi::CallbackInfo const &);
	/**
	 * @param string the device id
	 * @param DeviceType the device type
	 * @param string the session id
	 * @returns the new mute flag
	 */
	Napi::Boolean GetAudioSessionMute(Napi::CallbackInfo const &);
} // namespace SoundMixer