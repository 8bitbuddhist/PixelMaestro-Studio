TEMPLATE	= subdirs
CONFIG		+= ordered
SUBDIRS		= src tests

# When compiling for Windows or WebAssembly, don't compile tests
win32|wasm {
SUBDIRS -= tests
}

app.depends		= src
test.depends	= tests
