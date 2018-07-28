TEMPLATE	= subdirs
CONFIG		+= ordered
SUBDIRS		= src tests

# When compiling for Windows, don't compile tests
win32 {
	SUBDIRS -= tests
}

app.depends		= src
test.depends	= tests
