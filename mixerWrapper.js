const SoundMixer = require("./build/Release/SoundMixer.node");
const { clamp } = require("lodash");


const EDATAFLOW_RENDER = 0, EDATAFLOW_CAPTURE = 1;

const GetDefaultRenderDevice = () => SoundMixer.GetDefaultDevice(EDATAFLOW_RENDER);
const GetDefaultCaptureDevice = () => SoundMixer.GetDefaultDevice(EDATAFLOW_CAPTURE);

/**
 * @returns {[{id: string, name: string, render: boolean}]} the list of all active devices
 */
const GetDevices = SoundMixer.GetDevices;

/**
 * 
 * @param {string} deviceId the device id
 * @returns {[{path: string, processIds: Number[]}]} the list of appNames
 */
const GetSessions = (deviceId) => {
	const knownPaths = {};
	const provider = [];
	SoundMixer.GetSessions(deviceId).forEach(({ path, id }) => {
		if (Object.keys(knownPaths).includes(path)) {
			provider[knownPaths[path]].processIds.push(id);
		} else {
			provider.push({ path, processIds: [id] });
			knownPaths[path] = provider.length - 1;
		}
	});

	return provider;
}

/**
 * @param {string} deviceId the device id
 * @param {boolean} mute the mute/unmute value for the device specified in device 
 * @returns {boolean} the mute value
 */
const SetEndpointMute = (deviceId, mute) => {
	return SoundMixer.SetEndpointMute(deviceId, mute);
}

/**
 * @param {string} deviceId the device id
 * @returns {float} the endpoint volume scalar
 */
const GetEndpointMute = (deviceId) => {
	return SoundMixer.GetEndpointMute(deviceId);
}

/**
 * @param {string} deviceId the device id
 */
const ToggleEndpointMute = (deviceId) => {
	const currentMute = SoundMixer.GetEndpointMute(deviceId);
	return SetEndpointMute(deviceId, !currentMute);
}

/**
 * @param {string} deviceId the device id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set
 */
const SetEndpointVolume = (deviceId, volume) => {
	const effectiveScalar = clamp(volume, .0, 1.0);
	return SoundMixer.SetEndpointVolume(deviceId, effectiveScalar);
}

/**
 * @param {string} deviceId the device id
 * @returns {float} the volume scalar ranged from 0 to 1 for the device
 */
const GetEndpointVolume = (deviceId) => {
	return SoundMixer.GetEndpointVolume(deviceId);
}

/**
 * 
 * @param {string} deviceId the device id
 * @param {float} deltaVolume - the volume delta ranged from -1 to 1
 * @returns {float} the new volume scalar
 */
const ChangeEndpointVolume = (deviceId, deltaVolume) => {
	const currentValue = SoundMixer.GetEndpointVolume(deviceId);
	const newScalar = clamp(currentValue + deltaVolume, .0, 1.0);
	return SoundMixer.SetEndpointVolume(deviceId, newScalar);
}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process ids
 * @param {boolean} mute the value to set for the specified session 
 */
const SetAppMute = (deviceId, processId, mute) => {
	return SoundMixer.SetAudioSessionMute(deviceId, processId, mute);
}

/**
 * 
 * @param {string} deviceId the device id
 * @param {int} processId - the session processId
 * @returns {boolean} the mute value
 */
const GetAppMute = (deviceId, processId) => {
	return SoundMixer.GetAppMute(deviceId, processId);

}

/**
 * 
 * @param {string} deviceId the device id
 * @param {int} processId - the processId of the audio session
 * @returns {boolean} - the new mute value for the specified audio session
 */
const ToggleAppMute = (deviceId, processId) => {
	const currentValue = SoundMixer.GetAudioSessionMute(deviceId, processId);
	return SoundMixer.SetAudioSessionMute(deviceId, processId, !currentValue);

}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set for the specified session 
 */
const SetAppVolume = (deviceId, processId, volume) => {
	return SoundMixer.SetAudioSessionVolume(deviceId, processId, clamp(volume, 0, 1.0));
}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @returns {float} - the app volume scalar
 */
const GetAppVolume = (deviceId, processId) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.GetAudioSessionVolume(deviceId, processId);

}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @param {float} deltaVolume the volume change ranged from -1.0 to 1.0 to add for the specified session
 * @returns {float} the new volume scalar
 */
const ChangeAppVolume = (device, processId, deltaVolume) => {
	const currentValue = SoundMixer.GetAudioSessionVolume(deviceId, processId);

	return SoundMixer.SetAppVolume(deviceId, processId, clamp(currentValue + deltaVolume, 0, 1.0));

}

module.exports = {

	GetDevices,
	GetDefaultCaptureDevice,
	GetDefaultRenderDevice,
	GetSessions,

	SetEndpointMute,
	GetEndpointMute,
	ToggleEndpointMute,

	SetEndpointVolume,
	GetEndpointVolume,
	ChangeEndpointVolume,

	SetAppMute,
	GetAppMute,
	ToggleAppMute,

	SetAppVolume,
	GetAppVolume,
	ChangeAppVolume
}