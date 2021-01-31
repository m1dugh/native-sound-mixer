const SoundMixer = require('../build/Release/SoundMixer.node');
const lodash = require("lodash");


const precision = 2;

const EDATAFLOW_RENDER = 0, EDATAFLOW_CAPTURE = 1;

/**
 * @returns {{id: string, name: string, render: boolean}} the default output device
 */
const GetDefaultRenderDevice = () => SoundMixer.GetDefaultDevice(EDATAFLOW_RENDER);
modules.exports.GetDefaultRenderDevice = GetDefaultRenderDevice;

/**
 * @returns {{id: string, name: string, render: boolean}} the default input device
 */
module.exports.GetDefaultCaptureDevice = () => SoundMixer.GetDefaultDevice(EDATAFLOW_CAPTURE);

/**
 * @returns {[{id: string, name: string, render: boolean}]} the list of all active devices
 */
module.exports.GetDevices = SoundMixer.GetDevices;

/**
 * 
 * @param {string} deviceId the device id
 * @returns {[{path: string, processIds: Number[]}]} the list of appNames
 */
module.exports.GetSessions = (deviceId) => {
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
module.exports.SetDeviceMute = (deviceId, mute) => {
	return SoundMixer.SetEndpointMute(deviceId, mute);
}

/**
 * @param {string} deviceId the device id
 * @returns {float} the endpoint volume scalar
 */
module.exports.GetDeviceMute = (deviceId) => {
	return SoundMixer.GetEndpointMute(deviceId);
}

/**
 * @param {string} deviceId the device id
 */
module.exports.ToggleDeviceMute = (deviceId) => {
	const currentMute = SoundMixer.GetEndpointMute(deviceId);
	return SoundMixer.SetEndpointMute(deviceId, !currentMute);
}

/**
 * @param {string} deviceId the device id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set
 */
module.exports.SetDeviceVolume = (deviceId, volume) => {
	const effectiveScalar = lodash.clamp(volume, .0, 1.0);
	return lodash.round(SoundMixer.SetEndpointVolume(deviceId, effectiveScalar), precision);
}

/**
 * @param {string} deviceId the device id
 * @returns {float} the volume scalar ranged from 0 to 1 for the device
 */
module.exports.GetDeviceVolume = (deviceId) => {
	return lodash.round(SoundMixer.GetEndpointVolume(deviceId), precision);
}

/**
 * 
 * @param {string} deviceId the device id
 * @param {float} deltaVolume - the volume delta ranged from -1 to 1
 * @returns {float} the new volume scalar
 */
module.exports.ChangeDeviceVolume = (deviceId, deltaVolume) => {
	const currentValue = SoundMixer.GetEndpointVolume(deviceId);
	const newScalar = lodash.clamp(currentValue + deltaVolume, .0, 1.0);
	return lodash.round(SoundMixer.SetEndpointVolume(deviceId, newScalar), precision);
}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process ids
 * @param {boolean} mute the value to set for the specified session 
 */
module.exports.SetAppMute = (deviceId, processId, mute) => {
	return SoundMixer.SetAudioSessionMute(deviceId, processId, mute);
}

/**
 * 
 * @param {string} deviceId the device id
 * @param {int} processId - the session processId
 * @returns {boolean} the mute value
 */
module.exports.GetAppMute = (deviceId, processId) => {
	return SoundMixer.GetAudioSessionMute(deviceId, processId);

}

/**
 * 
 * @param {string} deviceId the device id
 * @param {int} processId - the processId of the audio session
 * @returns {boolean} - the new mute value for the specified audio session
 */
module.exports.ToggleAppMute = (deviceId, processId) => {
	const currentValue = SoundMixer.GetAudioSessionMute(deviceId, processId);
	return SoundMixer.SetAudioSessionMute(deviceId, processId, !currentValue);

}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set for the specified session 
 */
module.exports.SetAppVolume = (deviceId, processId, volume) => {
	return lodash.round(SoundMixer.SetAudioSessionVolume(deviceId, processId, lodash.clamp(volume, 0, 1.0)), precision);
}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @returns {float} - the app volume scalar
 */
module.exports.GetAppVolume = (deviceId, processId) => {
	return lodash.round(SoundMixer.GetAudioSessionVolume(deviceId, processId), precision);

}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @param {float} deltaVolume the volume change ranged from -1.0 to 1.0 to add for the specified session
 * @returns {float} the new volume scalar
 */
module.exports.ChangeAppVolume = (deviceId, processId, deltaVolume) => {
	const currentValue = SoundMixer.GetAudioSessionVolume(deviceId, processId);

	return lodash.round(SoundMixer.SetAudioSessionVolume(deviceId, processId, lodash.clamp(currentValue + deltaVolume, 0, 1.0)), precision);

}

