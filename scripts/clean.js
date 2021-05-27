const fs = require("fs");
const path = require("path");
const ROOT_DIR = path.resolve(__dirname, "..");

try {
	fs.rmdirSync(path.resolve(ROOT_DIR, "dist"), {recursive: true});
	fs.rmdirSync(path.resolve(ROOT_DIR, "build"), {recursive: true});
}catch(err) {
	console.error(err)
}