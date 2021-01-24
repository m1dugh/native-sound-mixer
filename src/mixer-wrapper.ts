import SoundMixer from 'sound-mixer';
import lodash from "lodash";
import Device from './model/device';


const EDATAFLOW_RENDER = 0, EDATAFLOW_CAPTURE = 1;

/**
 * @returns {{id: string, name: string, render: boolean}} the default output device
 */
export const getDefaultRenderDevice = (): Device => SoundMixer.GetDefaultDevice(EDATAFLOW_RENDER);

/**
 * @returns {{id: string, name: string, render: boolean}} the default input device
 */
export const getDefaultCaptureDevice = (): Device => SoundMixer.GetDefaultDevice(EDATAFLOW_CAPTURE);

/**
 * @returns {[{id: string, name: string, render: boolean}]} the list of all active devices
 */
export const getDevices: () => Device[] = SoundMixer.GetDevices;

/**
 * 
 * @param {string} deviceId the device id
 * @returns {[{path: string, processIds: Number[]}]} the list of appNames
 */
export const getSessions = (deviceId: string) => {
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
export const setDeviceMute = (deviceId, mute) => {
	return SoundMixer.SetEndpointMute(deviceId, mute);
}

/**
 * @param {string} deviceId the device id
 * @returns {float} the endpoint volume scalar
 */
export const getDeviceMute = (deviceId) => {
	return SoundMixer.GetEndpointMute(deviceId);
}

/**
 * @param {string} deviceId the device id
 */
export const toggleDeviceMute = (deviceId) => {
	const currentMute = SoundMixer.GetEndpointMute(deviceId);
	return SoundMixer.SetEndpointMute(deviceId, !currentMute);
}

/**
 * @param {string} deviceId the device id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set
 */
export const setDeviceVolume = (deviceId, volume) => {
	const effectiveScalar = lodash.clamp(volume, .0, 1.0);
	return SoundMixer.SetEndpointVolume(deviceId, effectiveScalar);
}

/**
 * @param {string} deviceId the device id
 * @returns {float} the volume scalar ranged from 0 to 1 for the device
 */
export const getDeviceVolume = (deviceId) => {
	return SoundMixer.GetEndpointVolume(deviceId);
}

/**
 * 
 * @param {string} deviceId the device id
 * @param {float} deltaVolume - the volume delta ranged from -1 to 1
 * @returns {float} the new volume scalar
 */
export const changeDeviceVolume = (deviceId, deltaVolume) => {
	const currentValue = SoundMixer.GetEndpointVolume(deviceId);
	const newScalar = lodash.clamp(currentValue + deltaVolume, .0, 1.0);
	return SoundMixer.SetEndpointVolume(deviceId, newScalar);
}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process ids
 * @param {boolean} mute the value to set for the specified session 
 */
export const setAppMute = (deviceId, processId, mute) => {
	return SoundMixer.SetAudioSessionMute(deviceId, processId, mute);
}

/**
 * 
 * @param {string} deviceId the device id
 * @param {int} processId - the session processId
 * @returns {boolean} the mute value
 */
export const getAppMute = (deviceId, processId) => {
	return SoundMixer.GetAudioSessionMute(deviceId, processId);

}

/**
 * 
 * @param {string} deviceId the device id
 * @param {int} processId - the processId of the audio session
 * @returns {boolean} - the new mute value for the specified audio session
 */
export const toggleAppMute = (deviceId, processId) => {
	const currentValue = SoundMixer.GetAudioSessionMute(deviceId, processId);
	return SoundMixer.SetAudioSessionMute(deviceId, processId, !currentValue);

}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set for the specified session 
 */
export const setAppVolume = (deviceId, processId, volume) => {
	return SoundMixer.SetAudioSessionVolume(deviceId, processId, lodash.clamp(volume, 0, 1.0));
}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @returns {float} - the app volume scalar
 */
export const getAppVolume = (deviceId, processId) => {
	return SoundMixer.GetAudioSessionVolume(deviceId, processId);

}

/**
 * @param {string} deviceId the device id
 * @param {int} processId the session process id
 * @param {float} deltaVolume the volume change ranged from -1.0 to 1.0 to add for the specified session
 * @returns {float} the new volume scalar
 */
export const changeAppVolume = (deviceId, processId, deltaVolume) => {
	const currentValue = SoundMixer.GetAudioSessionVolume(deviceId, processId);

	return SoundMixer.SetAudioSessionVolume(deviceId, processId, lodash.clamp(currentValue + deltaVolume, 0, 1.0));

}

