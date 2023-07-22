'use strict';
const path = require('path');
const bindings = require('node-gyp-build')(path.join(__dirname, '..'));

const moduleCache = {};
const modules = {};

const suffix = {
  'linux':  '.so',
  'linux2': '.so',
  'sunos':  '.so',
  'solaris':'.so',
  'freebsd':'.so',
  'openbsd':'.so',
  'darwin': '.dylib',
  'mac':    '.dylib',
  'win32':  '.dll'
}[process.platform];

function load(filepath) {
    if (filepath && typeof filepath === 'string' && filepath.indexOf(suffix) === -1) {
        const prefix = process.platform === 'win32' ? '' : 'lib';
        filepath = path.join(path.dirname(filepath), `${prefix}${path.basename(filepath)}${suffix}`);
    }
    if (!(filepath in moduleCache)) {
        const module_name = bindings.load(filepath);
        moduleCache[filepath] = new Proxy({}, {
            get: function(classCache, className) {
                if (!(className in classCache)) {
                    classCache[className] = bindings.loadType(`${module_name}.${className}`);
                }
                return classCache[className];
            }
        });
        modules[module_name] = moduleCache[filepath];
    }
    return moduleCache[filepath];
}

function ref(x) {
    return [x];
}

function unref(box) {
    return box[0]
}

module.exports.modules = modules;
module.exports.load = load;
module.exports.$ref = ref;
module.exports.$unref = unref;
module.exports.declaration = bindings.declaration;
