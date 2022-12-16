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

export declare class Device {
	private constructor();
	public volume: VolumeScalar;
	public mute: boolean;
	public balance: VolumeBalance;
	public readonly name: string;
	public readonly type: DeviceType;
	public readonly sessions: AudioSession[];

    public on(ev: string, callback: any): Number
    public removeListener(callback: any): boolean
}

export enum AudioSessionState {
	ACTIVE = 0,
	INACTIVE = 1,
	EXPIRED = 2
}

export enum DeviceType {
	CAPTURE = 1,
	RENDER = 0
}

export type VolumeScalar = number

export declare class AudioSession {
	private constructor();
	public volume: VolumeScalar;
	public balance: VolumeBalance;
	public mute: boolean;
	public readonly name: string;
	public readonly appName: string;
}

export declare type SoundMixer = {
	devices: Device[];
	getDefaultDevice(type: DeviceType): Device;
}

export default sMixerModule.SoundMixer
