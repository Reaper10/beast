BEAST & BSE
===========

[![License LGPL-2.1+](http://testbit.eu/~timj/pics/license-lgpl-2-1+.svg)](https://github.com/tim-janik/beast/blob/master/COPYING)
[![Build Status](https://travis-ci.org/tim-janik/beast.svg)](https://travis-ci.org/tim-janik/beast)

# DESCRIPTION

Beast is a digital synthesizer and music creation system. It has support
for Linux Audio Plugins
([LADSPA](http://wikipedia.org/wiki/LADSPA)),
multi-track editing, unlimited undo, real-time synthesis, MIDI and various
free audio codecs.
Bse is the Beast Sound Engine, a library providing the synthesis and
audio functions used by Beast.

* For a full description, visit the project page:
	http://beast.testbit.eu

* To submit bug reports and feature requests, visit:
	https://github.com/tim-janik/beast/issues


# REQUIREMENTS

Beast is a soft realtime application which needs elevated CPU scheduling
priorities to avoid audio glitches and drop outs. For this purpose, it
installs a small uid wrapper which acquires nice level -20 for the
synthesis threads and then immediately drops privileges.
For Linux kernels of the 2.6.x series and later, this enables the
low-latency scheduling behavior needed to avoid audio artefacts.

In order to build release tarballs, `GnomeCanvas`, `Ogg/Vorbis`,
`libflac` and `npm` and `libfluidsynth` are required.
Support for MP3 files is optional and requires `libmad` (MPEG audio
decoder library) when compiling Beast.
Compilation requires `g++-5.2.1` or later and a recent Linux
distribution like Ubuntu-18.04.


# INSTALLATION

In short, Beast needs to be built and installed with:

	./configure
	make
	make check         # run simple unit tests
	make install
	make installcheck  # run audio tests

Note that Beast has to be fully installed to function properly, and that
the binaries built in the launchers/ directory have to be installed with
permissions of the root user.

# BINARY PACKAGES

New source code pushed to the Beast repository is automatically built
and tested via Travis-CI. Ever so often we create release candidate
packages and later release packages after a stabilization phase.
Some of the release candidates and the stable versions are provided
as binary packages. Refer to the website for download links.
