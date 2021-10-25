=======
Modules
=======

Introduction
============

This document discusses the barebone CommonJS-based module framework
built into Duktape 1.x (moved into an optional extra in Duktape 2.x):

* ECMAScript modules are defined using the CommonJS module format:

  - http://wiki.commonjs.org/wiki/Modules/1.1.1

  - Duktape supports also ``module.exports`` and a few Duktape specific
    properties (``module.filename`` and ``module.name``)

* The user must provide a *module search function* which locates a module
  corresponding to a resolved module ID, and can register module symbols
  directly and/or return module source code as a string.  To remain portable
  and unaware of file systems and such, Duktape does not provide a default
  module search function.

* C modules, static or DLL-based, can be implemented on top of the module
  search function by user code.  There is no built-in C module support in
  the main Duktape library to avoid portability issues for exotic platforms.
  However, there is a recommended convention which works on most platforms
  and allows both static and DLL loading, see ``c-module-convention.rst``.

Using modules from ECMAScript code (require)
============================================

Duktape provides a global ``require()`` function which allows a module to be
loaded based on a module identifier::

  var lib = require('package/lib')

The module identifier may contain forward slashes to provide some hierarchy.
There are both absolute and relative identifiers::

  // absolute identifier, resolves to 'a/b' everywhere
  var mod = require('a/b');

  // relative identifier, resolves to 'package/c' inside package/lib
  var mod = require('./c');

  // relative identifier, resolves to 'foo/xyz' inside foo/bar/quux
  var mod = require('../xyz');

Relative paths are resolved starting from the absolute root of the current
module, i.e. the module calling ``require()``, see separate section on the
resolution algorithm.

A module with a certain resolved identifier is loaded only once per a global
environment: ``Duktape.modLoaded`` keeps track of modules which are fully or
partially loaded, mapping a resolved identifier to the module's ``module``
table.  If a required module has already been (fully or partially) loaded,
``require()`` will simply return ``module.exports``.  (But note that if a
module is still being loaded and assigns to ``module.exports``, the return
value might be different for a later ``require()`` call.)

If a module has not yet been registered to ``Duktape.modLoaded``, Duktape
registers an initial ``module`` table to ``Duktape.modLoaded``.  It then
calls ``Duktape.modSearch()``, a module search function which must be
provided by the user (there is no default)::

  Duktape.modSearch = function (id, require, exports, module) {
    // ...
  };

The identifier given to the ``modSearch()`` function is a fully resolved,
absolute identifier.

If the search function cannot locate a module based on its identifier, it is
expected to throw an error.  If a module is found, the search function can
register symbols directly to 'exports' (this is used to implement C modules),
and can also return a string to be used as the module ECMAScript source code::

  /* An actual implementation would usually scan and return module sources
   * e.g. from the filesystem or a compressed source pack.  This example
   * illustrates the return value options.
   */

  Duktape.modSearch = function (id, require, exports, module) {
    if (id === 'foo') {
      /* 'foo' is a native module, register symbols to 'exports' in a
       * platform specific way.
       */
      return;  // undefined: no ECMAScript source
    }
    if (id === 'bar') {
      /* 'bar' is a pure ECMAScript module. */
      return 'exports.hello = function () { print("Hello world from bar!"); };';
    }
    if (id === 'quux') {
      /* 'quux' is a mixed C/ECMAScript module.  C code provides the
       * exports.rawFunc() binding while ECMAScript code implements
       * a safe variant on top of that.
       */
      return 'exports.func = function () {\n' +
             '    try { exports.rawFunc(); } catch (e) { print(e); }\n' +
             '};\n';
    }

    /* If a module is not found, an error must be thrown. */
    throw new Error('cannot find module: ' + id);
  };

The ``module`` value is registered to ``Duktape.modLoaded`` before the
module search function is called (this was changed in Duktape 1.3), so
that circular requires are properly supported for both ECMAScript and C
modules.

The user ``Duktape.modSearch()`` function encapsulates functionality such as
module search paths and file I/O.  This is a nice division of labor as it
encapsulates practically all of the platform dependent parts of module
loading, while keeping the user callback unaware of almost every other
aspect of module loading.  Using stub module search functions is also easy,
which is good for testing.

ECMAScript modules follow the CommonJS format, e.g. ``package/lib`` could
be represented by the source file::

  exports.add = function (a, b) {
    return a + b;
  };

Replacing ``module.exports`` is also supported as of Duktape 1.3.  Although
``module.exports`` is not part of CommonJS, it's widely supported by other
module frameworks, and allows code like::

  module.exports = function adder(a, b) {
    return a + b;
  };

CommonJS instructs that modules should be evaluated with certain bindings
in force.  Duktape currently implements the CommonJS requirements by simply
wrapping the module code inside some footer/header code::

  (function (require, exports, module) {
    /* module code here */
  })

So the example module would become::

  (function (require, exports, module) {
    exports.add = function (a, b) {
      return a + b;
    };
    // return value is ignored
  })

When evaluated, the expression results in a function object (denoted ``F``)
which is then called (more or less) like::

  var exports = {};
  var module = {
    exports: exports,   /* initial value, may be replaced by user */
    id: 'package/lib'
  };
  F.call(exports,                 /* exports also used as 'this' binding */
         require,                 /* require method */
         exports,                 /* exports */
         module);                 /* module */

A few notes:

* The return value of this call is ignored.

* If either modSearch() or the module code throws an error, the module is
  de-registered from ``Duktape.modLoaded`` and the error is then re-thrown
  (this was changed in Duktape 1.3 to match the behavior of e.g. Node.js).

* The first argument is a new function object whose underlying native function
  is the same as the global ``require()`` function.  This fresh function is
  needed to facilitate resolution of relative module identifiers: relative
  identifers are resolved relative to the current module.  The resolved
  absolute identifier of the current module is tracked in ``require.id``.
  Native code can then pick up the resolution path from the current function
  object.

* The third argument provides the module with its own, resolved identifier.
  The value in ``module.id`` is guaranteed to be in absolute form, and resolve
  to the module itself if required from any other module.

Module caching when module loading fails
========================================

The "module" table of a module is registered to ``Duktape.modLoaded`` just
before calling either modSearch() or the wrapped module function.  This
registration must be done before running the module function because there
may be circular requires which require that cache entry to be present.

But what should be done with the modLoaded entry if the module function
throws an error?  CommonJS doesn't specify what to do in this situation.
Duktape 1.2 would keep the partial module in modLoaded, so that if you
tried to reload the module, the partial module would be returned directly.

Since Duktape 1.3 the modLoaded entry will be removed on module load error
so that it's possible to try to load the module again.  This matches Node.js
behavior.  See the test case:

- ``test-commonjs-module-load-error.js``

CommonJS module identifier resolution
=====================================

CommonJS specifies that identifier terms must be "camelCase":

* http://wiki.commonjs.org/wiki/Modules/1.1#Module_Identifiers

Some interpret this to mean that e.g. a dash character is not allowed.
Such an interpretation seems counterproductive because e.g. filenames
often contain dashes, underscores, etc.  Duktape allows terms to contain
any characters (including non-ASCII and white space) except that:

* A term must not begin with a period (``.``) to simplify resolution.
  Such terms are rejected.

* A term cannot contain a forward slash, which (of course) gets
  interpreted as a separator.

* A term cannot contain a U+0000 character.  Such terms are currently
  not rejected.  Instead, they terminate the resolution as if the
  requested identifier had ended.

If user code wishes to impose further limits, the module search function
can check a resolved identifier and throw an error if it is not of a
desirable form.

Logger names and tracebacks
===========================

Logger name defaulting uses the calling function's ``fileName`` property.
The ``fileName`` of the internal module wrapper function is set to the
resolved module identifier to make the logger default name come out right.
Application code can control logger name by overwriting ``module.filename``
in modSearch().

Tracebacks show both ``name`` and ``fileName`` of the internal wrapper
function.  Application code can control these by overwriting
``module.filename`` and/or ``module.name``.

module.exports
==============

NodeJS allows the default ``exports`` value to be changed by the module being
loaded; it can even be replaced e.g. by a function (it's normally an object
value).  To change the value, the module must assign to ``module.exports``
which initially has the same value as ``exports``:

* http://timnew.github.io/blog/2012/04/20/exports_vs_module_exports_in_node_js/

Duktape supports ``module.exports`` since Duktape 1.3, see:

* ``test-commonjs-module-exports-repl.js``

module.filename and module.name
===============================

The ``module.filename`` and ``module.name`` properties are Duktape specific
but also supported by e.g. Node.js.  They allow modSearch() to control the
``.fileName`` and ``.name`` properties of the module wrapper function used
to implement module loading.  This is useful because they appear in e.g.
tracebacks for errors created from the module.  See discussion in:
https://github.com/svaarala/duktape/pull/639.

The properties are missing by default.  If modSearch() doesn't set them,
the module wrapper function .fileName defaults to the full resolved module ID
(e.g. ``foo/bar``) and .name defaults to the last component of the resolved
module ID (e.g. ``bar``).

The capitalization of ``module.filename`` is chosen to match Node.js (see
https://nodejs.org/api/modules.html#modules_module_filename); the form
``.fileName`` is used elsewhere in Duktape.  See discussion in:
https://github.com/svaarala/duktape/pull/660.

C modules and DLLs
==================

Recommended convention
----------------------

``c-module-convention.rst`` describes a recommended convention for defining
an init function for a C module.  The convention allows a C module to be
initialized manually when using static linking, or as part of loading the
module from a DLL.

The recommendation is in no way mandatory and you can easily write a module
loader with your own conventions (see below).  However, modules following
the recommended convention will be easier to share between projects.

Implementing a C module / DLL loader
------------------------------------

The user provided module search function can be used to implement DLL support.
Simply load the DLL based on the module identifier, and call some kind of init
function in the DLL to register module symbols into the 'exports' table given
to the module loader.

Mixed C/ECMAScript modules are also possible by first registering symbols
provided by C code into the 'exports' table, and then returning the ECMAScript
part of the module.  The ECMAScript part can access the symbols provided by C
code through the shared 'exports' table.

As of Duktape 1.3, the ``module`` table is registered to ``Duktape.modLoaded``
before the module search function is called, so that circular requires are now
supported for C modules too.

Limitations:

* There is no automatic mechanism to know when a DLL can be unloaded from
  memory.  Tracking the reachability of the exports table of the module
  (e.g. through a finalizer) is **not** enough because other modules can
  copy references to individual exported values.

Background
==========

Module frameworks
-----------------

ECMAScript has not traditionally had a module mechanism.  In browser
environments a web page can load multiple script files in a specific
order, each of them introducing more global symbols.  This is not very
elegant because the order of loading must be correct in case any code
runs during loading.  Several module mechanisms have since been created
for the browser environment to make writing modular ECMAScript easier.
Similar needs also exist in non-browser environments and several mechanisms
have been defined.

References summarizing several module frameworks:

* http://addyosmani.com/writing-modular-js/

* http://wiki.commonjs.org/wiki/Modules

Module loading APIs or "formats":

* Asynchronous Module Definition (AMD) API:

  - https://github.com/amdjs/amdjs-api/wiki/AMD

* CommonJS:

  - http://wiki.commonjs.org/wiki/Modules/1.1.1

  - https://github.com/joyent/node/blob/master/lib/module.js

  - https://github.com/commonjs/commonjs/tree/master/tests/modules

  - http://requirejs.org/docs/commonjs.html

  - http://dailyjs.com/2010/10/18/modules/

* NodeJS, more or less CommonJS:

  - http://nodejs.org/docs/v0.11.13/api/modules.html

  - https://nodejs.org/api/modules.html

* ES2015:

  - http://www.ecma-international.org/ecma-262/6.0/index.html#sec-modules

AMD is optimized for the web client side, and requires callback based
asynchronous module loading.  This model is not very convenient for
server side programming, or fully fledged application programming which
is more natural with Duktape.

CommonJS module format is a server side module mechanism which seems most
appropriate to be the default Duktape mechanism.

Some NodeJS tests
=================

This section illustrates some NodeJS module loader features, as it's nice
to align with NodeJS behavior when possible.

Assignments
-----------

Test module::

  // test.js
  var foo = 123;     // not visible outside
  bar = 234;         // assigned to global object
  this.quux = 345;   // exported from module
  exports.baz = 456; // exported from module

Test code::

  > var t = require('./test');
  undefined
  > console.log(JSON.stringify(t));
  {"quux":345,"baz":456}
  undefined
  > console.log(bar);
  234

Future work
===========

Ability to load modules from C code
-----------------------------------

For instance, implement something like::

  // Pushes the 'exports' table of 'foo/bar' module to the stack.
  duk_require_module(ctx, "foo/bar");

This is not a high priority thing as one can simply::

  duk_eval_string(ctx, "require('foo/bar')");

Eval invokes the compiler which is not ideal, but modules are usually
imported during initialization so this should rarely matter.

Better C module support
-----------------------

* Provide a default DLL loading helper for at least POSIX and Windows.

Module unloading support
------------------------

Currently modules cannot be unloaded: once loaded, they're registered to
``Duktape.modLoaded`` permanently, which keeps the exported object permanently
reachable (unless removed manually).  Adding a finalizer to the exports table
is not a solution: another module might hold a reference to a specific symbol
within the module but not the exports table itself, e.g.::

  var helloFunc = require('hello').func;

Collecting a module exports table and executing some unload code is not
trivial.  Just removing an unused exports object probably requires weak
reference support.

Isolating a module from the global object
-----------------------------------------

Currently ``this`` is bound to ``exports`` so writes through ``this`` do
not pollute globals.  Variable and function declarations also currently
go to the module wrapper function and do not pollute globals.  However,
plain assignments do write to globals, and reads not matching identifiers
declared in scope are read from globals::

    fooBar = 123;  // if 'fooBar' not in scope, write to global
    print(barFoo); // if 'barFoo' not in scope, read from global

Lua-like module loader
----------------------

The lowest level module mechanism could also be similar to what Lua does.
A module would be cached as in CommonJS so that it would only be loaded
once per global context.  Modules could be loaded with a user callback
which takes a module ID and returns the loaded module object (same as the
``exports``) value to be registered into the module cache.

The upside of this approach is flexibility: most of the CommonJS module
mechanism can be implemented on top of this.

One downside is that the module loading mechanism would not be a common one
and most users would need to implement or borrow a standard module loader.
Another downside is that a Lua-like mechanism doesn't deal with circular
module loading while the CommonJS one does (to some extent).
