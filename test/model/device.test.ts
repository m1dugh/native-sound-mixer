import { random, clamp } from "lodash";
import "../../dist/@types/sound-mixer.d.ts"
import SoundMixer, { Device, DeviceType } from "../../dist/sound-mixer.js"



describe("set device volume", () => {
	let device: Device;
	let originalVolume: number;

	beforeAll(() => {
		const devices = SoundMixer.devices;
		device = devices[random(0, devices.length - 1)];
		originalVolume = device.volume;
	});

	it("should set volume to 0", () => {
		device.volume = 0
		expect(device.volume).toBe(0)
	})

	it("should add .3 to the volume", () => {
		const expectedVolume = clamp(device.volume + .3, .0, 1.)
		device.volume += .3
		expect(device.volume.toFixed(1)).toBe(expectedVolume.toFixed(1));
	})

	it("should turn the volume off", () => {
		device.volume -= 1.
		expect(device.volume).toBe(.0)
	})

	it("should turn the volume max power", () => {
		device.volume += 1.
		expect(device.volume).toBe(1.)
	})

	afterAll(() => {
		device.volume = originalVolume;
	})
})

describe("set device mute", () => {
	let device: Device;
	let originalMute: boolean;
	beforeAll(() => {
		const devices = SoundMixer.devices;
		device = devices[random(0, devices.length - 1)];
		originalMute = device.mute;
	});

	it("should set the mute value to true", () => {
		device.mute = true
		expect(device.mute).toBe(true)
	})

	it("should set the mute value to false", () => {
		device.mute = false;
		expect(device.mute).toBeFalsy()
	})

	it("should change the mute state", () => {
		device.mute = true;
		expect(device.mute).toBe(true)

		device.mute = false;
		expect(device.mute).toBeFalsy()
	})

	afterAll(() => {
		device.mute = originalMute;
	})
})

describe("get sessions", () => {

	let device: Device;
	beforeEach(() => {
		const devices = SoundMixer.devices.filter(d => d.sessions.length > 0);
		if (devices.length > 0)
			device = devices[random(0, devices.length - 1)]
	});

	it("should find sessions", () => {
		if (device !== undefined)
			expect(device.sessions.length).toBeGreaterThanOrEqual(1);
	});

	for (const d of SoundMixer.devices) {
		it("should call sessions", () => {
			expect(typeof d.sessions).toBe(typeof []);
		})
	}
})

describe("volume balance", () => {

	let device: Device;
	beforeEach(() => {
		const devices = SoundMixer.devices.filter(d => d.balance.stereo);
		if (devices.length > 0)
			device = devices[random(0, devices.length - 1)]
	});

	it("should set balance", () => {
		const previous = device.balance;


		device.balance = { right: 1, left: 1 }
		expect(device.balance).toEqual({ right: 1, left: 1, stereo: true })

		device.balance = previous;
	})

	it("should clamp balance down", () => {
		const previous = device.balance;
		device.balance = { right: -1, left: -1 };
		expect(device.balance).toEqual(previous)
		device.balance = previous;
	})

	it("should clamp balance up", () => {
		const previous = device.balance;
		device.balance = { right: 100, left: 100 }
		expect(device.balance).toEqual(previous)
		device.balance = previous;
	})

})
