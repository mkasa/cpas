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

If you are using Intel C++ compiler, you may want to use icpc
instead of g++ to gain more speed. -icpc option adds a special
comment that makes cpas use icpc.

When your editor is emacs, -emacs might be also useful to enforce
c++ mode. I prefer to use -fskel, which is equivalent to giving
all the above options.


Debug macro
===========

Here is an exmaple.::

	#include <cpas_debug.h>
	
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

Eval
====

cpas has a built-in eval function, similarly to other scripting
languages like Perl or Ruby. First, you need to include eval.h,
which is included in the distribution.::

	#include <cpas_eval.h>

Next, you create an instance of Eval class, then calling
eval_function member function.::

	Eval ev;
	Eval::Handle hd = ev.eval_function(
		"extern \"C\" bool is_record_match(const char* line_str) { "
		"    const int val = atoi(line_str);"
		"    return val < 80;"
		"}");

Eval::Handle is a handle to access to the eval'ed functions.
You may write mutiple functions in the string. You may define
global variables as well. Calling eval_function invokes g++,
which compiles the string into a dynamic linking library.
Note that demangling is not automatic, so if you intend to eval
C++ functions without extern "C", you may need the mangled name
of the symbols.

You can get the pointer to the functions or variables by calling
get_function.::

	typedef bool (MATCH_CHECK_FUNC_TYPE)(const char *);
	MATCH_CHECK_FUNC_TYPE* func;
	hd.get_function("is_record_match", &func);
	// Here you got the pointer in func.

You should use typedef to avoid messy pointer declaration, but
still you can get by without typedef if you want. get_function
member is a template function so you do not need to cast by
yourself. That is why I did not return the pointer by return value.

If the returned pointer is NULL, it indicates an error.::

	if(func == NULL) {
		fprintf(stderr, "ERROR: could not find such a function\n");
		exit(1);
	}

You can call the eval'ed function as normal functions.::

	if(func("70")) {
		printf("70 < 80\n");
	}


License
=======

This software is distributed under modified BSD license
(http://www.opensource.org/licenses/bsd-license.php)

Several header files (string_piece.h and dependencies) are
derived from the Chromium project, and their license is
shown at the head of the header files. 

