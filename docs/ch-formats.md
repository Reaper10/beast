# File Formats and Conventions

## The overall structure of BSE files

A `*.bse` file is an Ascii file with occasional binary appendix and contains
various data proccessed by the BSE library.
The readable (Ascii) part of a `.bse` file is held in lisp syntax and mostly
build up out of (keyword value) pairs.

### Identification Tag

*[NOTE: currently (1999-12-21) the identification tag support is disabled,
        and subject for general overhaul. A new implementation is likely
        to come soon and will probably (roughly) follow this specification.]*

Each file has a special identification tag in the first line:

~~~scheme
(BSE-Data V1 0000000004);               -*- scheme -*-
 ^^^^^^^^ ^^ ^^^^^^^^^^
 |        |  |
 |        |  ten-digit octal flag
 |        |
 |        version tag
 |
 BSE data-identifier
~~~

and is matched against a regexp pattern (in grep -E syntax):

	^\(BSE-Data\ V1\ [0-7]{10}\);.*$

Note that the `';'` at the end of the right parenthesis is actually part
of the expression and needs to directly follow the right parenthesis,
whereas the space inbetween the `';'` and the line end `'\n'` can contain
any number of, or even no characters at all.
The ten-digit octal value is a short hand flag to identify the various
kinds of data held in the current file. Its value corresponds to the
enum definiton BseIoDataFlags in <bse/bseenums.h>.

### Important keywords

Main statement keywords - prefixed by a left parenthesis, to start a certain
statement - are:

BseSNet
:	A source network specification, containing
	all the sources (filters) contained in the network,
	their properties and associated link structures.

BseSong
:	Holding a song definition including all of a
	song's data, e.g. instrument definitions,
	patterns, effects, etc.

BseSample
:	A sample definition, usually including references
	to certain binary blocks that have their binary
	data appended to the file.

Substatement keywords depend on the object (main statement) they apply to,
and should be mostly self explanatory ;)

### Binary Appendices

One object invariant substatement keyword is "`BseBinStorageV0`", it's
syntax is as follows:

~~~scheme
( BseBinStorageV0 <UOFFSET> <ENDIANESS>:<N_BITS> <ULENGTH> )
~~~

`<UOFFSET>`
: offset within the binary appendix for this block's data

`<ENDIANESS>`
: either 'L' or 'B' for respectively little or big endianess

`<N_BITS>`
: number of bits per (endianess-conformant) value stored in this block

`<ULENGTH>`
: number of bytes contained in this block


Binary appendices are stored after the ascii part of a .bse file. A NUL
(`'\0'`) byte character unambiguously marks the end of the ascii portion.
Directly following this NUL byte the binary appendix starts out,
containing binary data as specified through the various BseBinStorageV0
statements within the ascii portion.
Files without binary appendix may not contain the terminating NUL byte.
