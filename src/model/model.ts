

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

	constructor(
		/**
		 * @member
		 * the linked {@link Device} id
		 */
		protected readonly deviceId: string,
		/**
		 * @member
		 * the unique and persistent session id
		 */
		public readonly id: string,
		/**
		 * @member
		 * the path of the app linked to the audio session
		 */
		public readonly path: string,
		/**
		 * @member
		 * the state of the audio session
		 */
		public readonly state: AudioSessionState
	) { }

	/**
	 * @property
	 * the volume of the device
	 */
	abstract set volume(volume: VolumeScalar);

	abstract get volume(): VolumeScalar;

	/**
	 * @property
	 * the mute flag
	 */
	abstract get mute(): boolean;
	abstract set mute(mute: boolean);

}


export abstract class Device {

	constructor(
		/**
		 * @member
		 * the unique and persistent id of the device
		 */
		public readonly id: string,
		/**
		 * @member
		 * the friendly name of the device
		 */
		public readonly name: string,
		/**
		 * @member
		 * the type of the device
		 */
		public readonly type: DeviceType
	) { }

	/**
	 * @property
	 * the audio sessions bound to the device
	 */
	abstract get sessions(): AudioSession[];

	/**
	 * @property
	 * the volume of the audio session
	 */
	abstract get volume(): VolumeScalar;
	abstract set volume(volumeToSet: VolumeScalar);

	/**
	 * @property
	 * the mute flag of the audio session
	 */
	abstract get mute(): boolean;
	abstract set mute(mute: boolean);

	/**
	 * 
	 * @param {string} id - the audio session id
	 * @returns {AudioSession} the audio session matching id
	 */
	abstract getSessionById(id: string): AudioSession | undefined;
}