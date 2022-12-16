const SoundMixer = require("../").default
const { memoryUsage } = require("process")


let rss = memoryUsage().rss
const deltaRss = {
    old: 0,
    value: 0
}
const heap = memoryUsage().heapTotal

const time = (() => {
	const t = new Date().getTime()
	return () => new Date().getTime() - t
})()


setTimeout(() => rss = memoryUsage().rss, 100);

setInterval(() => {
	for (const d of SoundMixer.devices) {
		d.mute = d.mute
		d.volume = d.volume
		for (const s of d.sessions) {
			s.volume = s.volume
			s.mute = s.mute
		}
	}

}, 1);

setInterval(() => {
    deltaRss.old = deltaRss.value
    deltaRss.value = memoryUsage().rss
}, 1000)


setInterval(() => {
	console.clear()
	const usage = memoryUsage()
	console.log("time\trss increase\tcurrent rss\theap increase\theap usage\taverage rss growth\trss growth/1s")
	console.log(`${Math.round(time() / 1000)}s\t${((usage.rss - rss) / Math.pow(1024, 2)).toFixed(1)}mB\t\t${Math.round(usage.rss / 1024)}kB\t\t${((usage.heapTotal - heap) / Math.pow(1024, 2)).toFixed(3)}mB\t\t${Math.round((usage.heapUsed / usage.heapTotal) * 100)}%\t\t${((usage.rss - rss) * 1000 / (1024 * time())).toFixed(1)}kB/s\t\t${Math.round((deltaRss.value - deltaRss.old) / 1024)}kB`)
}, 100)
