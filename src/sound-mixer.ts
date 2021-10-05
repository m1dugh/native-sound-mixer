
import * as os from "os";

const sMixerModule: { SoundMixer: SoundMixer } = (() => {

	const getModule = (platform: "macos" | "win" | "linux", arch: string | undefined = undefined) => require(`${__dirname}/addons/${platform}-sound-mixer${arch ? "_" + arch : ""}.node`)
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

export declare class Device {
	private constructor();
	public volume: VolumeScalar;
	public mute: boolean;
	public readonly name: string;
	public readonly type: DeviceType;
	public readonly sessions: AudioSession[];
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
	public mute: boolean;
	public readonly name: string;
	public readonly appName: string;
}

export declare type SoundMixer = {
	devices: Device[];
	getDefaultDevice(type: DeviceType): Device;
}

export default sMixerModule.SoundMixer