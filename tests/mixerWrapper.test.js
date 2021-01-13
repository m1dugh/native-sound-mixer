const mixerWrapper = require("../mixerWrapper");

describe('test native sound mixer', () => {

	let deviceId;

	beforeAll(() => {
		deviceId = mixerWrapper.GetDefaultRenderDevice().id;
	});

	test('get devices', () => {
		const devices = mixerWrapper.GetDevices();
		expect(devices.length).toBeGreaterThan(0);
	});



	describe('set/get endpoint volume', () => {

		let state;

		beforeAll(() => {
			console.log("volume for device is", mixerWrapper.GetEndpointVolume(deviceId));
			state = mixerWrapper.GetEndpointVolume(deviceId);
		});

		describe("for value < 0", () => {
			test("set should return 0", () => {
				const value = mixerWrapper.SetEndpointVolume(deviceId, -1);
				expect(value).toBe(0);
			});

			test("get should return 0", () => {
				expect(mixerWrapper.GetEndpointVolume(deviceId)).toBe(0);
			})
		})

		describe("for value > 1", () => {
			test("set should return 1", () => {
				const value = mixerWrapper.SetEndpointVolume(deviceId, 2);
				expect(value).toBe(1);
			});

			test("get should return 1", () => {
				expect(mixerWrapper.GetEndpointVolume(deviceId)).toBe(1);
			})
		});

		describe("for valid value", () => {

			test("set should return .5", () => {
				const value = mixerWrapper.SetEndpointVolume(deviceId, .5);
				expect(value).toBe(.5);
			});

			test("get should return .5", () => {
				expect(mixerWrapper.GetEndpointVolume(deviceId)).toBe(.5);
			})
		})

		afterAll(() => {
			mixerWrapper.SetEndpointVolume(deviceId, state);
		})
	})

	describe('set/get endpoint mute', () => {

		let state;

		beforeAll(() => {
			state = mixerWrapper.GetEndpointMute(deviceId);
		});

		describe("for mute value", () => {
			test("set should return 0", () => {
				const value = mixerWrapper.SetEndpointMute(deviceId, true);
				expect(value).toBe(true);
			});

			test("get should return 0", () => {
				expect(mixerWrapper.GetEndpointMute(deviceId)).toBe(true);
			})
		})

		describe("for toggle mute", () => {

			test("should return opposite value of current", () => {
				const currentValue = mixerWrapper.GetEndpointMute(deviceId);
				expect(mixerWrapper.ToggleEndpointMute(deviceId)).toBe(!currentValue);
			});

		})

		afterAll(() => {
			mixerWrapper.SetEndpointMute(deviceId, state);
		})
	})

	test('get audio sessions', () => {
		const audioSessions = mixerWrapper.GetSessions(deviceId);
		console.log("audioSessions: ", audioSessions);
		expect(audioSessions.length).toBeGreaterThanOrEqual(0);
	});
});