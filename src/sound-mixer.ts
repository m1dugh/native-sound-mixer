import * as os from "os"

/*
 */
type platform = "macos" | "win" | "linux";
type arch = string | undefined

const sMixerModule: { SoundMixer: SoundMixer } = (() => {

	const getModule = (platform: platform, arch: arch = undefined) => {
        const archString = arch ? `_${arch}` : ""
        const path = 
            `${__dirname}/addons/${platform}-sound-mixer${archString}.node`
        const res = require(path)
        return res;
    }

	const platform = os.platform()
	const arch = os.arch()
	if (platform === "win32") {
		if (arch === "x32" || arch === "x64" || arch === "ia32") {
			return getModule("win");
		}
	} else if (platform == "linux") {
		if (arch === "x32" || arch === "x64" || arch === "ia32")
			return getModule("linux");
	}

	throw new Error("could not get the binary file")

})()

/**
 *  An interface indicating the balance level of each channel on a stereo device or channel.
 */
export interface VolumeBalance {
    /**
     *  right: The volume for the right channel.
     */
	right: VolumeScalar;

    /**
     *  left: The volume for the left channel.
     */
	left: VolumeScalar;

    /**
     *  stereo: A flag indicating whether the owner is stereo.
     */
	stereo?: Boolean;
}

/**
 *  A class that represents an actual physical or virtual audio device.
 */
export declare class Device {

	private constructor();

    /**
     *  The current volume of the device.
     *  @remarks Writing to this property changes the volume of the device.
     */
	public volume: VolumeScalar;

    /**
     *  A flag indicating the mute state of a device.
     *  @remarks Writing to this property changes the mute state of the device.
     */
	public mute: boolean;

    /**
     *  The stereo balance of the device if available.
     */
	public balance: VolumeBalance;

    /**
     *  The name of the device.
     *  @readonly
     */
	public readonly name: string;

    /**
     *  The type of the device (input or output).
     *  @readonly
     */
	public readonly type: DeviceType;

    /**
     *  Returns the audio sessions bound to the device.
     *  @readonly 
     */
	public readonly sessions: AudioSession[];

    /**
     *  @param {string} ev - The type of event to subscribe to. 
     *  It can be either `volume`, or `mute`.
     *
     *  @param {function} callback - The callback to run when the event is
     *  triggered.
     *
     *  @returns {number} - The id of the registered callback used to 
     *  remove the listener.
     *
     *  @remarks Triggering a volume change, or mute change in a callback can
     *  cause the listeners to self trigger, leading to an infinite trigger
     *  loop.
     *
     *  @see {@link Device.removeListener | removing a listener}
     */
    public on(ev: string, callback: any): number

    /**
     *  @param {string} ev - The type of event to remove the listener of. 
     *
     *  @param {number} handler - The identifier of the registered callback
     *  to be removed.
     *
     *  @returns {boolean} - Whether the callback was unregistered or not.
     *
     *  @see {@link Device.on | registering a listener}
     */
    public removeListener(ev: string, handler: number): boolean
}

/**
 *  An enum giving the state of an {@link AudioSession}
 *  @enum
 */
export enum AudioSessionState {
	ACTIVE = 0,
	INACTIVE = 1,
	EXPIRED = 2
}

/**
 *  An enum giving the type of {@link Device}, it can be either `input` or 
 *  `output`.
 *  @enum
 */
export enum DeviceType {
    
    /**
     *  Input mode.
     */
	CAPTURE = 1,

    /**
     *  Output mode.
     */
	RENDER = 0
}

/**
 *  A floating-point number from `0.0` to `1.0` returning the rate of the
 *  volume. `1.0` represents the most important value, that is, the 
 *  {@link Device} or the {@link AudioSession} is at `100%` volume.
 *  `0.0` means that the volume captures or renders no sound at all.
 *  @typedef {number}
 */
export type VolumeScalar = number

/**
 *  A class representing an audio session, that is the sound rendered or
 *  captured by one application.
 *  @class
 */
export declare class AudioSession {
    /**
     *  @private
     */
	private constructor();

    /**
     *  The volume of the {@link AudioSession}
     *  @see {@link Device.volume}
     */
	public volume: VolumeScalar;

    /**
     *  The volume balance of the {@link AudioSession} if stereo.
     *  @see {@link Device.balance}.
     */
	public balance: VolumeBalance;

    /**
     *  The mute flag of the {@link AudioSession}.
     *  @see {@link Device.mute}.
     */
	public mute: boolean;

    /**
     *  The name of the {@link AudioSession}.
     *  @remarks Depending on the `C++` background implementation,
     *  this attribute might not behave the same on all distributions.
     *  @readonly
     */
	public readonly name: string;

    /**
     *  The path to the application using the {@link AudioSession}.
     *  @remarks Depending on the `C++` background implementation, 
     *  this attribute might not behave the same on all distributions.
     *  @readonly
     */
	public readonly appName: string;
}

/**
 *  The sound mixer object containing all
 *  the devices.
 *  @typedef {Object} SoundMixer
 */
export declare type SoundMixer = {

    /**
     *  The list of active {@link Device | devices} when the property is
     *  read.
     *  @static
     */
	devices: Device[];

    /**
     *  Gets the default device of the given type.
     *  @param {DeviceType} type - The type of the device to be retrieved.
     *  @returns {Device} - The default {@link Device} if found, null
     *  otherwise.
     *  @static
     */
	getDefaultDevice(type: DeviceType): Device;
}

/**
 *  The actual {@link SoundMixer} object.
 */
const soundMixer: SoundMixer = sMixerModule.SoundMixer


export default soundMixer
