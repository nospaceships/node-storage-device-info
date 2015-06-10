
var assert = require("assert")
var os = require("os");

var storage = require("../");

var validPath   = os.platform() == "win32" ? "c:\\"  : "/";
var invalidPath = os.platform() == "win32" ? "XX:\\" : "invalid-path";

describe("getPartitionSpace()", function() {
	describe("with a valid path", function() {
		it("should be successful", function(done) {
			storage.getPartitionSpace(validPath, function(error, space) {
				assert.equal(error, null);
				assert(space.totalMegaBytes);
				assert(space.freeMegaBytes);
				done();
			});
		});
	});
	describe("with an invalid path", function() {
		it("should fail", function(done) {
			storage.getPartitionSpace(invalidPath, function(error, space) {
				assert(error instanceof Error);
				done();
			});
		});
	});
});
