=====================
cpas: CPp As a Script
=====================

What is cpas?
=============

Script languages like Perl or Python are useful and convenient.
However, don't you sometimes feel they are too slow?

I often handle large files of nearly terabytes, being frustrated by
slowness of Perl or Python. Nice things with script languages are,
for example, (1) no compilation is needed, (2) a single file may
contain a code and a document at once and thus easy to manage.

When I found Tiny C Compiler (TCC, http://bellard.org/tcc/),
it was pretty surprising to me. It had C-script mode, with which you
can write like this::

	#!/usr/local/bin/tcc -run
	#include <stdio.h>

	int main() 
	{
	    printf("Hello World\n");
	    return 0;
	}

This script is compiled on the fly. Although the compiled code is
a bit slower than GCC, the compilation time is sufficiently short.

CPp as a script (cpas) follows this idea and applied it to C++
scripting.

cpas does not compile source code on the fly; it would be better
but very difficult to implement. It simply invokes g++ and
the compiled code is stored in .cpas directory. .cpas directory
is created in the same directory as the script unless the
directory is not writable. If that is the case, the compiled code
is created under ~/.cpas; the file name of the compiled code is
determined by the SHA256 hash of the absolute path of the script
file, thus ensuring one-to-one mapping between the original
script and the compiled binary while avoiding name conflict
between different scripts with the same file name.

Compiled binaries are cached, so it is automatically recompiled
when the source code is modified. Note that this feature rely on
timestamp of files. The use of ntpd is strongly recommended on
a cluster environment.


Build & Install
===============

As usual for GNU tar balls, to configure, do as follows::

	$ ./configure

Then do make and make install,::

	$ make && make install

cpas uses waf, which is a build system written in python;
therefore you need python to configure and make cpas.


How to use
==========

Specify cpas in the shebang. chmod the script file appropriately.::

	#!/usr/local/bin/cpas
	#include <iostream>
	using namespace std;
	int main() {
	    cout << "Hello, World!" << endl;
	}

Then, you can use the script as if it is a Light-weight language
script such as Perl or Python script. Enjoy!


Advanced use
============

cpas outputs a script stub when -skel option is specified.
If you are going to use GNU getopt, use -getopt option together.
To emit a perldoc-style document, use -doc, then your script
becomes a document as well (i.e., try 'perldoc your_script'). 

If you prefer to see a stack dump when a program dies because of
segmentation fault, add -sdump when you create a skelton.

A special debug macro, which I developed for C++ debugging, is
installed with cpas. Please add -dmacro when you create a skelton
if you use the debug macro. For the debug macro, see the next
section for details.

When your editor is emacs, -emacs might be also useful to enforce
c++ mode. I prefer to use -fskel, which is equivalent to giving
all the above options.


Debug macro
===========

Here is an exmaple.::

	#include <debug.h>
	
	void your_function()
	{
	    int i = 1233;
	    // True, so nothing happens.
	    ASSERT(i == 1233);
	    int x = 1234;
	    // This assert fails. You'll see the message,
	    // and dump of the variable x.
	    ASSERT_WMD("x must be 1233", x == 1233, VARDUMP(x));
	}

License
=======

This software is distributed under modified BSD license
(http://www.opensource.org/licenses/bsd-license.php)

Several header files (string_piece.h and dependencies) are
derived from the Chromium project, and their license is
shown at the head of the header files. 

