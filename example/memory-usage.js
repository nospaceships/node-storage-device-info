
var storage = require ("../");

if (process.argv.length < 3) {
	console.log ("usage: " + process.argv[1] + " <path>");
	process.exit(-1);
}

function doCheck(dir) {
	storage.getPartitionSpace(dir, function (error, space) {
		if (error) {
			console.log (error.message);
		} else {
			console.dir(space);
			doCheck(dir);
		}
	});
}

doCheck(process.argv[2]);
