import {
	Device,
	AudioSession,
	DeviceType,
	VolumeScalar,
	AudioSessionState
} from "./model/model";

import { round } from "lodash";
import { arch, platform } from "os";

const ROUND_PRECISION = 3;

const sMixerModule = (() => {

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


const _safeWrap = (func: Function) => {
	try {
		return func();
	} catch (err) {
		if (err.name === "TypeError")
			throw new TypeError(err)
		throw new Error(err);
	}
}


class AudioSessionImpl extends AudioSession {

	get mute() {
		return _safeWrap(() => sMixerModule.GetAudioSessionMute(this.deviceId, this.deviceType, this.id))
	}

	get volume() {
		return round(_safeWrap(() => sMixerModule.GetAudioSessionVolume(this.deviceId, this.deviceType, this.id)), ROUND_PRECISION);
	}

	set mute(mute: boolean) {
		_safeWrap(() => sMixerModule.SetAudioSessionMute(this.deviceId, this.deviceType, this.id, mute));
	}

	set volume(volume: VolumeScalar) {
		_safeWrap(() => sMixerModule.SetAudioSessionVolume(this.deviceId, this.deviceType, this.id, volume));
	}

}

class DeviceImpl extends Device {

	get sessions() {

		const ids = new Set();
		return sMixerModule.GetSessions(this.id, this.type).map(({ id, path, state }: AudioSession) => new AudioSessionImpl(this.id, this.type, id, path, state))
			.filter(({ id }: AudioSession) => !ids.has(id) && ids.add(id));
	}

	get mute() {
		return _safeWrap(() => sMixerModule.GetDeviceMute(this.id, this.type));
	}

	get volume() {
		return round(_safeWrap(() => sMixerModule.GetDeviceVolume(this.id, this.type)), ROUND_PRECISION);
	}

	set mute(mute: boolean) {
		_safeWrap(() => sMixerModule.SetDeviceMute(this.id, this.type, mute));
	}

	set volume(volume: VolumeScalar) {
		_safeWrap(() => sMixerModule.SetDeviceVolume(this.id, this.type, volume));
	}

	getSessionById(id: string) {
		const sessions: AudioSession[] = this.sessions
			.filter((s: AudioSession) => id === s.id)
		if (sessions.length > 0) {
			return sessions[0]
		}
		return undefined;
	}

}

export default class SoundMixer {

	static get devices(): Device[] {
		return sMixerModule.GetDevices().map(({ id, name, type }: { id: string, name: string, type: number }) => new DeviceImpl(id, name, type))
	}

	static getDefaultDevice(type: DeviceType): Device {
		const { id, name, type: nativeType } = sMixerModule.GetDefaultDevice(type);
		return new DeviceImpl(id, name, nativeType);
	}

	static getDeviceById(id: string): Device | undefined {
		const devices: Device[] = sMixerModule.GetDevices().filter(({ id: devId }: Device) => devId === id);
		if (devices.length > 0)
			return devices[0];
		return undefined;
	}

}

export {
	Device,
	AudioSession,
	DeviceType,
	AudioSessionState,
	VolumeScalar
}