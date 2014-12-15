
# storage-device-info - [homepage][homepage]

Obtain information about storage devices.

This module is installed using [node package manager (npm)][npm]:

    # This module contains C++ source code which will be compiled
    # during installation using node-gyp.  A suitable build chain
    # must be configured before installation.
    
    npm install storage-device-info

It is loaded using the `require()` function:

    var storage = require ("storage-device-info");

Storage information can then be obtained:

    storage.getPartitionSpace("c:\\", function(error, space) {
       console.dir(space);
    });

    storage.getPartitionSpace("/opt", function(error, space) {
       console.dir(space);
    });

[homepage]: http://re-tool.org "Homepage"
[npm]: https://npmjs.org/ "npm"

# Using This Module

Currently this module exports only the function `getPartitionSpace()`.

## storage.getPartitionSpace(path, callback)

The `getPartitionSpace()` function obtains space utilisation information for
the partition on which the `path` exists.

For example, on Windows, the `c:\\temp` directory is located on the `c:\\`
partition, and so space utilisation information will be obtained for the `c:\\`
partition.

The `callback` function is called once space information has been obtained.
The following arguments will be passed to the `callback` function:

 * `error` - Instance of the `Error` class, or `null` if no error occurred
 * `space` - An object which will have the following attributes:
    * `totalMegaBytes`
    * `freeMegaBytes`

The following example obtains space information for the `/` partition on which
the `/opt` directory is located:

    storage.getPartitionSpace("/opt", function(error, space) {
        if (error) {
            console.log(error);
        } else {
            console.log("total: " + space.totalMegaBytes + "\n"
                + "free: " + space.freeMegaBytes + "\n");
        }
    });

# Example Programs

Example programs are included under the modules `example` directory.

# Bugs & Known Issues

None, yet!

Bug reports should be sent to <stephen.vickers.sv@gmail.com>.

# Changes

## Version 1.0.0 - 08/08/2014

 * Initial release

## Version 1.0.1 - 08/08/2014

 * Add more keywords to the `package.json` file

## Version 1.0.2 - 15/12/2014

 * Support MAC OS X

## Version 1.0.3 - 15/12/2014

 * Unused variables warnings seen during build on MAC OS X

# Roadmap

Suggestions and requirements should be sent to <stephen.vickers.sv@gmail.com>.

# License

Copyright (c) 2014 Stephen Vickers

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

# Author

Stephen Vickers <stephen.vickers.sv@gmail.com>
