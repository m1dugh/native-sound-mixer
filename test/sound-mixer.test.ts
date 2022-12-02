import SoundMixer, { DeviceType } from "../"

describe("sound mixer", () => {

	it("should get all devices", () => {
		const devices = SoundMixer.devices
		expect(devices.filter(({ type }) => type === DeviceType.RENDER).length).toBeGreaterThanOrEqual(1);
		expect(devices.filter(({ type }) => type === DeviceType.CAPTURE).length).toBeGreaterThanOrEqual(1);

	});

	it("should get the default render device", () => {
		const device = SoundMixer.getDefaultDevice(DeviceType.RENDER)
		expect(typeof device).toBe("object")
		expect(device.type).toBe(DeviceType.RENDER)
	})

	it("should get the default capture device", () => {
		const device = SoundMixer.getDefaultDevice(DeviceType.CAPTURE)
		expect(typeof device).toBe("object")
		expect(device.type).toBe(DeviceType.CAPTURE)
	})

})