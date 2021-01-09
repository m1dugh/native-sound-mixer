const mixerWrapper = require("../mixerWrapper");

const defaultDevice = null;

describe('set/get endpoint volume', () => {

	let state;

	beforeAll(() => {
		state = mixerWrapper.GetEndpointVolume(defaultDevice);
	});

	describe("for value < 0", () => {
		test("set should return 0", () => {
			const value = mixerWrapper.SetEndpointVolume(defaultDevice, -1);
			expect(value).toBe(0);
		});

		test("get should return 0", () => {
			expect(mixerWrapper.GetEndpointVolume(defaultDevice)).toBe(0);
		})
	})

	describe("for value > 1", () => {
		test("set should return 1", () => {
			const value = mixerWrapper.SetEndpointVolume(defaultDevice, 2);
			expect(value).toBe(1);
		});

		test("get should return 1", () => {
			expect(mixerWrapper.GetEndpointVolume(defaultDevice)).toBe(1);
		})
	});

	describe("for valid value", () => {

		test("set should return .5", () => {
			const value = mixerWrapper.SetEndpointVolume(defaultDevice, .5);
			expect(value).toBe(.5);
		});

		test("get should return .5", () => {
			expect(mixerWrapper.GetEndpointVolume(defaultDevice)).toBe(.5);
		})
	})

	afterAll(() => {
		mixerWrapper.SetEndpointVolume(defaultDevice, state);
	})
})

describe('set/get endpoint mute', () => {

	let state;

	beforeAll(() => {
		state = mixerWrapper.GetEndpointMute(defaultDevice);
	});

	describe("for mute value", () => {
		test("set should return 0", () => {
			const value = mixerWrapper.SetEndpointMute(defaultDevice, true);
			expect(value).toBe(true);
		});

		test("get should return 0", () => {
			expect(mixerWrapper.GetEndpointMute(defaultDevice)).toBe(true);
		})
	})

	describe("for toggle mute", () => {

		test("should return opposite value of current", () => {
			const currentValue = mixerWrapper.GetEndpointMute(defaultDevice);
			expect(mixerWrapper.ToggleEndpointMute(defaultDevice)).toBe(!currentValue);
		});

	})

	afterAll(() => {
		mixerWrapper.SetEndpointMute(defaultDevice, state);
	})
})

test('get audio sessions', () => {
	const audioSessions = mixerWrapper.GetSessions(defaultDevice);
	expect(audioSessions.length).toBeGreaterThan(0);
});