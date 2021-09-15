
import { arch, platform } from "os";

const sMixerModule: { SoundMixer: SoundMixer } = (() => {

	const getModule = (platform: "macos" | "win" | "linux", arch: string | undefined = undefined) => require(`${__dirname}/addons/${platform}-sound-mixer${arch ? "_" + arch : ""}.node`)
	if (platform() === "win32") {
		if (arch() === "x32" || arch() === "x64") {
			return getModule("win");
		}
	} else if (platform() == "linux") {
		if (arch() === "x32" || arch() === "x64")
			return getModule("linux");
	}

	throw new Error("could not get the binary file")

})()

declare class Device {
	public volume: VolumeScalar;
	public mute: boolean;
	public readonly name: string;
	public readonly type: DeviceType;
	public readonly sessions: AudioSession[];
}

enum AudioSessionState {
	ACTIVE = 0,
	INACTIVE = 1,
	EXPIRED = 2
}

enum DeviceType {
	CAPTURE = 1,
	RENDER = 0
}

export type VolumeScalar = number

declare class AudioSession {
	public volume: VolumeScalar;
	public mute: boolean;
	public readonly name: string;
	public readonly appName: string;
}

declare type SoundMixer = {
	devices: Device[];
	getDefaultDevice(type: DeviceType): Device;
}

export default sMixerModule.SoundMixer

export {
	DeviceType,
	Device,
	AudioSession,
	AudioSessionState
}