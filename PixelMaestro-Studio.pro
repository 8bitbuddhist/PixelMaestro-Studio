TEMPLATE	= subdirs
CONFIG		+= ordered
SUBDIRS		= src tests
app.depends		= src
test.depends	= tests