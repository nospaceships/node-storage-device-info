
var storage = require ("../");

if (process.argv.length < 3) {
	console.log ("usage: " + process.argv[1] + " <path>");
	process.exit(-1);
}

storage.getPartitionSpace(process.argv[2], function (error, space) {
	if (error) {
		console.log (error.message);
	} else {
		console.dir(space);
	}
});
