
export enum DeviceType {
	RENDER = 0,
	CAPTURE = 1,
	ALL = 2
}

export enum AudioSessionState {
	INACTIVE,
	ACTIVE,
	EXPIRED
}

/** a float representing the volume (1.0 is full volume, .0 is no volume) */
export type VolumeScalar = number;

export abstract class AudioSession {

	constructor(protected readonly deviceId: string,
		public readonly id: string,
		public readonly path: string,
		public readonly state: AudioSessionState
	) { }

	abstract set volume(volume: VolumeScalar);
	abstract get volume(): VolumeScalar;
	// abstract changeVolume(deltaVolume: VolumeScalar): VolumeScalar;

	abstract get mute(): boolean;
	abstract set mute(mute: boolean);
	// abstract toggleMute(): boolean;

}

export abstract class Device {
	constructor(public readonly id: string, public readonly name: string, public readonly type: DeviceType) { }

	abstract get sessions(): AudioSession[];

	abstract get volume(): VolumeScalar;
	abstract set volume(volumeToSet: VolumeScalar);
	// abstract changeVolume(deltaVolume: VolumeScalar): VolumeScalar;

	abstract get mute(): boolean;
	abstract set mute(mute: boolean);
	// abstract toggleMute(): boolean;
}