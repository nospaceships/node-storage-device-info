
var storage = require ("./build/Release/storage");

var wrap;

exports.getPartitionSpace = function(path, cb) {
	try {
		if (! wrap)
			wrap = new storage.DeviceInfoWrap();
		wrap.getPartitionSpace(path, cb);
	} catch(error) {
		cb(error);
	}
}
