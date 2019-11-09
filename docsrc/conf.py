# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'PixelMaestro Studio'
copyright = '2019, 8bitbuddhist'
author = '8bitbuddhist'

# The full version, including alpha/beta/rc tags
release = '0.50.2'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['sphinx.ext.extlinks',
              'breathe',
              'exhale']

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# Create extlink to PixelMaestro docs
extlinks = {
    'pmdocs': ('https://8bitbuddhist.github.io/PixelMaestro/%s', 'pmdocs'),
    'pmarduino': ('https://github.com/8bitbuddhist/PixelMaestro/blob/master/examples/arduino/%s', 'pmarduino')
}


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
html_favicon = 'images/logo.png'

# Configure the breathe extension
breathe_projects = {
    "PixelMaestroStudio": "./doxygen/xml"
}
breathe_default_project = "PixelMaestroStudio"

# Configure exhale
exhale_args = {
    "containmentFolder":    "./api",
    "rootFileName":         "root.rst",
    "rootFileTitle":        "PixelMaestro Studio API",
    "doxygenStripFromPath": "..",
    "createTreeView":          True,
    "exhaleExecutesDoxygen":   True,
    "exhaleDoxygenStdin":   "INPUT = ../src/"
}

primary_domain = 'cpp'
highlight_language = 'cpp'
