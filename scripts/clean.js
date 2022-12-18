const fs = require("fs");
const path = require("path");
const ROOT_DIR = path.resolve(__dirname, "..");

try {
	fs.rmSync(path.resolve(ROOT_DIR, "dist"), {recursive: true, force: true});
}catch(err) {
	console.error(err)
}
