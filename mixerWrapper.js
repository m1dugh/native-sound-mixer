const SoundMixer = require("./build/Release/SoundMixer.node");
const { clamp } = require("lodash");

/*
 * device like : 
 * {
 *    deviceType,
 *    deviceRole,
 * }
 */
const EDATAFLOW_RENDER = 0
const EDATAFLOW_CAPTURE = 1

const EDATAFLOW_DEFAULT = EDATAFLOW_RENDER;

const EROLE_CONSOLE = 0
const EROLE_MULTIMEDIA = 1
const EROLE_COMMUNICATIONS = 2;

const EROLE_DEFAULT = EROLE_CONSOLE;


const getDeviceAttributes = (device) => ({
	deviceType: (device?.deviceType || EDATAFLOW_DEFAULT),
	deviceRole: (device?.deviceRole || EROLE_DEFAULT)
});

/**
 * 
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to fetch the sessions on
 * @returns {[{path: string, processIds: Number[]}]} the list of appNames
 */
const GetSessions = (device) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	const knownPaths = {};
	const provider = [];
	SoundMixer.GetSessions(deviceType, deviceRole).forEach(({ path, id }) => {
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
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to mute
 * @param {boolean} mute the mute/unmute value for the device specified in device 
 * @returns {boolean} the mute value
 */
const SetEndpointMute = (device, mute) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	SoundMixer.SetEndpointMute(deviceType, deviceRole, mute);

	return SoundMixer.GetEndpointMute(deviceType, deviceRole);
}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to mute
 * @returns {float} the endpoint volume scalar
 */
const GetEndpointMute = (device) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.GetEndpointMute(deviceType, deviceRole);
}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to toggle the mute on
 */
const ToggleEndpointMute = (device) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	const currentMute = SoundMixer.GetEndpointMute(deviceType, deviceRole);
	return SetEndpointMute(device, !currentMute);
}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to change the volume on
 * @param {float} volume the volume scalar ranged from 0 to 1 to set
 */
const SetEndpointVolume = (device, volume) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	const effectiveScalar = clamp(volume, .0, 1.0);
	return SoundMixer.SetEndpointVolume(deviceType, deviceRole, effectiveScalar);
}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to change the volume on
 * @returns {float} the volume scalar ranged from 0 to 1 for the device
 */
const GetEndpointVolume = (device) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.GetEndpointVolume(deviceType, deviceRole);

}

/**
 * 
 * @param {{deviceType?: Number, deviceRole?: Number}} - device the device to change the volume on
 * @param {float} deltaVolume - the volume delta ranged from -1 to 1
 * @returns {float} the new volume scalar
 */
const ChangeEndpointVolume = (device, deltaVolume) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	const currentValue = SoundMixer.GetEndpointVolume(deviceType, deviceRole);
	const newScalar = clamp(currentValue + deltaVolume, .0, 1.0);
	return SoundMixer.SetEndpointVolume(deviceType, deviceRole, newScalar);
}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to set the mute app on
 * @param {int} processId the session process ids
 * @param {boolean} mute the value to set for the specified session 
 */
const SetAppMute = (device, processId, mute) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.SetAudioSessionMute(deviceType, deviceRole, processId, mute);
}

/**
 * 
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to fetch the session mute value on 
 * @param {int} processId - the session processId
 * @returns {boolean} the mute value
 */
const GetAppMute = (device, processId) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.GetAppMute(deviceType, deviceRole, processId);

}

/**
 * 
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to toggle the mute app value on
 * @param {int} processId - the processId of the audio session
 * @returns {boolean} - the new mute value for the specified audio session
 */
const ToggleAppMute = (device, processId) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	const currentValue = SoundMixer.GetAudioSessionMute(deviceType, deviceRole, processId);
	return SoundMixer.SetAudioSessionMute(deviceType, deviceRole, processId, !currentValue);

}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to set the volume app value on
 * @param {int} processId the session process id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set for the specified session 
 */
const SetAppVolume = (device, processId, volume) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.SetAudioSessionVolume(deviceType, deviceRole, processId, clamp(volume, 0, 1.0));
}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to set the volume app value on
 * @param {int} processId the session process id
 * @returns {float} - the app volume scalar
 */
const GetAppVolume = (device, processId) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	return SoundMixer.GetAudioSessionVolume(deviceType, deviceRole, processId);

}

/**
 * @param {{deviceType?: Number, deviceRole?: Number}} device the device to set the volume app value on
 * @param {int} processId the session process id
 * @param {float} deltaVolume the volume change ranged from -1.0 to 1.0 to add for the specified session
 * @returns {float} the new volume scalar
 */
const ChangeAppVolume = (device, processId, deltaVolume) => {
	const { deviceType, deviceRole } = getDeviceAttributes(device);
	const currentValue = SoundMixer.GetAudioSessionVolume(deviceType, deviceRole, processId);

	return SoundMixer.SetAppVolume(deviceType, deviceRole, processId, clamp(currentValue + deltaVolume, 0, 1.0));

}

module.exports = {

	DeviceRoles: {
		EROLE_CONSOLE,
		EROLE_MULTIMEDIA,
		EROLE_COMMUNICATIONS,
		EROLE_DEFAULT

	},
	DeviceDataFlows: {
		EDATAFLOW_RENDER,
		EDATAFLOW_CAPTURE,
		EDATAFLOW_DEFAULT
	},
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