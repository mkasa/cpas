#ifndef _STACK_DUMP_HEADER_
#define _STACK_DUMP_HEADER_

#include <cstdio>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

class GDB_On_SEGV
{
	 static char arg_buffer1[256];
	 static char lbuf[PATH_MAX];
	 static char stdoutbuf[256];
	 static char *p;

	 static struct sigaction old_sa; // consider the case in which the stack is corrupted.
	 static struct sigaction sa;     // we allocate it staticly.

	 static void waitgdb(int fd) {
		  int c = 1;
		  int i;
		  while((i = read(fd, stdoutbuf, sizeof(stdoutbuf))) > 0) {
			   for(int j = 0; j < i; j++) {
					switch(stdoutbuf[j]) {
					case '\n':
						 c = 1;
						 break;
					case '(':
						 if(c == 1) c++; else c = 0;
						 break;
					case 'g':
						 if(c == 2) c++; else c = 0;
						 break;
					case 'd':
						 if(c == 3) c++; else c = 0;
						 break;
					case 'b':
						 if(c == 4) c++; else c = 0;
						 break;
					case ')':
						 if(c == 5) c++; else c = 0;
						 break;
					default:
						 c = 0;
					}
					if(c == 6) {
						 write(1, stdoutbuf, i);
						 return;
					}
			   }
			   write(1, stdoutbuf, i);
		  }
	 }

public:
	 static void gdb_handler(int dummy) {
		  int pid;
		  if((pid = fork()) != 0) {
			   int status;
			   waitpid(pid, &status, 0);
			   write(2, "End\n", 4);
			   exit(1);
		  } else {
			   int p[2];
			   int fp[2];
			   if(pipe(p) != 0) {
					write(2, "pipe (p) failed\n", 16);
					exit(2);
			   }
			   if(pipe(fp) != 0) {
					write(2, "pipe (fp) failed\n", 17);
					exit(2);
			   }
			   if((pid = fork()) != 0) {
					close(p[0]);
					close(fp[1]);
					waitgdb(fp[0]);
					write(1,    "bt full\n", 8);
					write(p[1], "bt full\n", 8);
					waitgdb(fp[0]);
					write(1,    "up 3\n", 5);
					write(p[1], "up 3\n", 5);
					waitgdb(fp[0]);
					write(1,    "l\n", 2);
					write(p[1], "l\n", 2);
					waitgdb(fp[0]);
					write(1,    "disas\n", 6);
					write(p[1], "disas\n", 6);
					waitgdb(fp[0]);
					write(1,    "quit\n", 5);
					write(p[1], "quit\n", 5);
					// sleep(3);
					exit(3);
			   } else {
					close(p[1]);
					close(fp[0]);
					dup2(p[0], 0);
					dup2(fp[1], 1);
					execlp("gdb", "gdb", "-q", lbuf, arg_buffer1, NULL);
			   }
		  }
	 }

	 GDB_On_SEGV(char* argv0) {
		  p = realpath(argv0, lbuf);
		  if(p == NULL) {
			   fprintf(stderr, "Failed to expand the executable path '%s'\n", argv0);
			   exit(1);
		  }
		  // fprintf(stderr, "My executable path is %s\n", p);
		  sprintf(arg_buffer1, "%d", getpid());
		  memset(&sa,     0, sizeof(sa));
		  memset(&old_sa, 0, sizeof(old_sa));
		  sa.sa_handler = gdb_handler;
		  sa.sa_flags = SA_RESETHAND;
		  sigemptyset(&sa.sa_mask);
		  sigaction(SIGSEGV, &sa, &old_sa);
	 }

	 ~GDB_On_SEGV() {
		  sigaction(SIGSEGV, &old_sa, NULL);
	 }
};

char GDB_On_SEGV::arg_buffer1[256];
char GDB_On_SEGV::lbuf[PATH_MAX];
char GDB_On_SEGV::stdoutbuf[256];
char *GDB_On_SEGV::p;

struct sigaction GDB_On_SEGV::old_sa;
struct sigaction GDB_On_SEGV::sa;

#endif // #ifndef _STACK_DUMP_HEADER_

