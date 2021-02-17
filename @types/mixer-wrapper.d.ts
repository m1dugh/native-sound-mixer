import Device, {AudioSession} from './model/device';
/**
 * @returns {Device} the default output device
 */
export declare const GetDefaultRenderDevice: () => Device;
/**
 * @returns {Device} the default input device
 */
export declare const GetDefaultCaptureDevice: () => Device;
/**
 * @returns {[Device]} the list of all active devices
 */
export declare const GetDevices: () => Device[];
/**
 *
 * @param {string} deviceId the device id
 * @returns {[AudioSession]} the list of appNames
 */
export declare const GetSessions: (deviceId: string) => AudioSession[];
/**
 * @param {string} deviceId the device id
 * @param {boolean} mute the mute/unmute value for the device specified in device
 * @returns {boolean} the mute value
 */
export declare const SetDeviceMute: (deviceId: string, mute: boolean) => boolean;
/**
 * @param {string} deviceId the device id
 * @returns {float} the device mute value after set
 */
export declare const GetDeviceMute: (deviceId: string) => boolean;
/**
 * @param {string} deviceId the device id
 * @returns {boolean} the mute value after toggle
 */
export declare const ToggleDeviceMute: (deviceId: string) => boolean;
/**
 * @param {string} deviceId the device id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set
 * @returns {float} the device volume scalar after set
 */
export declare const SetDeviceVolume: (deviceId: string, volume: number) => number;
/**
 * @param {string} deviceId the device id
 * @returns {float} the volume scalar ranged from 0 to 1 for the device
 */
export declare const GetDeviceVolume: (deviceId: string) => number;
/**
 *
 * @param {string} deviceId the device id
 * @param {float} deltaVolume - the volume delta ranged from -1 to 1
 * @returns {float} the device volume scalar after set
 */
export declare const ChangeDeviceVolume: (deviceId: string, deltaVolume: number) => number;
/**
 * @param {string} deviceId the device id
 * @param {string} sessionId the session id
 * @param {boolean} mute the value to set for the specified session
 * @returns {boolean} the mute value set for specified app
 */
export declare const SetAppMute: (deviceId: string, sessionId: string, mute: boolean) => boolean;
/**
 *
 * @param {string} deviceId the device id
 * @param {string} sessionId - the session id
 * @returns {boolean} the mute value
 */
export declare const GetAppMute: (deviceId: string, sessionId: string) => boolean;
/**
 *
 * @param {string} deviceId the device id
 * @param {string} sessionId - the ID of the audio session
 * @returns {boolean} - the mute value set for the specified app
 */
export declare const ToggleAppMute: (deviceId: string, sessionId: string) => boolean;
/**
 * @param {string} deviceId the device id
 * @param {string} sessionId the session id
 * @param {float} volume the volume scalar ranged from 0 to 1 to set for the specified session
 * @returns {float} the volume scalar set for app
 */
export declare const SetAppVolume: (deviceId: string, sessionId: string, volume: number) => number;
/**
 * @param {string} deviceId the device id
 * @param {string} sessionId the session id
 * @returns {float} - the app volume scalar set
 */
export declare const GetAppVolume: (deviceId: string, sessionId: string) => number;
/**
 * @param {string} deviceId the device id
 * @param {string} sessionId the session id
 * @param {float} deltaVolume the volume change ranged from -1.0 to 1.0 to add for the specified session
 * @returns {float} the volume scalar set for app
 */
export declare const ChangeAppVolume: (deviceId: string, sessionId: string, deltaVolume: number) => number;
//# sourceMappingURL=mixer-wrapper.d.ts.map
