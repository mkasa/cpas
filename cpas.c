#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <getopt.h>

static char *cache_dir_name = ".cpas";

char executable_file_name[PATH_MAX];
char my_cache_dir[PATH_MAX + 10];
char source_code_path[PATH_MAX + 18];
char executable_path[PATH_MAX + 18];
char compile_options[1024];
static int flag_skelton   = 0;
static int flag_getopt    = 0;
static int flag_perldoc   = 0;
static int flag_emacs_var = 01;

void generate_cache_file_names()
{
  snprintf(source_code_path, sizeof(source_code_path), "%s/%s.cpp", my_cache_dir, executable_file_name);
  snprintf(executable_path,  sizeof(executable_path),  "%s/%s",     my_cache_dir, executable_file_name);
}

int need_to_compile(char *original_script)
{
  struct stat ebuf, sbuf;
  if(stat(executable_path, &ebuf) != 0) {
    return 1; // it doesn't exist
  }
  if(stat(original_script, &sbuf) != 0) {
    fprintf(stderr, "error: cannot stat the source file '%s'\n", original_script);
    exit(7);
  }
  if(sbuf.st_mtime <= ebuf.st_mtime)
    return 0;
  return 1; // it's old!
}

void convert_script(char *original_script)
{
  FILE *ifp;
  FILE *ofp;
  char buf[256];
  if((ifp = fopen(original_script, "r")) == NULL) {
    fprintf(stderr, "error: cannot open script '%s'\n", original_script);
    exit(3);
  }
  if((ofp = fopen(source_code_path, "w")) == NULL) {
    fprintf(stderr, "error: cannot open output cache script '%s'\n", source_code_path);
    exit(4);
  }
  while(!feof(ifp)) {
    char *l = fgets(buf, sizeof(buf), ifp);
    if(l == NULL) break;
    if(buf[0] == '#' && buf[1] == '!') {
      fprintf(ofp, "//%s\n", buf); // for not to change the number of lines.
      continue;
    }
    if(buf[0] == '/' && buf[1] == '/') {
      char *p;
      p = strstr(buf + 2, "opt:");
      if(p != NULL) {
	int l;
	l = strlen(compile_options);
	if(0 < l && compile_options[l - 1] != ' ' && l + 1 < sizeof(compile_options)) {
	  compile_options[l] = ' ';
	  compile_options[l] = '\0';
	}
	strncat(compile_options, p + 4, sizeof(compile_options));
	continue;
      }
    }
    fprintf(ofp, "%s\n", l);
  }
  fclose(ofp);
  fclose(ifp);
}

void compile_script()
{
  int status;
  pid_t child;
  if((child = fork()) == 0) {
    char buf[2048];
    int mystatus;
    sprintf(buf, "g++ %s -o %s %s\n", source_code_path, executable_path, compile_options);
    mystatus = system(buf);
    if(mystatus == -1) abort();
    exit(WEXITSTATUS(status));
  }
  wait(&status);
  if(!WIFEXITED(status) || WEXITSTATUS(status) == 6) {
    exit(6);
  }
}

void generate_executable_file_name(char *script_path)
{
  const int CRCF = 0x04C11DB7;
  const int LEFT = 0x04000000;
  int       crc  =  0x1234567;
  char canon_path[PATH_MAX];
  char *p;
  int i;
  char *r;
  r = realpath(script_path, canon_path);
  if(r == NULL) {
    fprintf(stderr, "error: cannot find '%s'\n", script_path);
    exit(5);
  }
  for(p = canon_path; *p; p++) {
    for(i = 7; i >= 0; i--) {
      crc = (crc << 1) | ((*p >> i) & 1);
      if(crc & LEFT) crc ^= CRCF;
    }
  }
  sprintf(executable_file_name, "%08X", crc);
}

void check_for_cache_dir(char *script_path)
{
  strncpy(my_cache_dir, script_path, sizeof(my_cache_dir));
  {
    char *p = my_cache_dir + strlen(my_cache_dir);
    while(my_cache_dir < p) {
      if(*(p - 1) == '/') break;
      p--;
    }
    strcpy(p, cache_dir_name);
  }
  struct stat buf;
  if(access(my_cache_dir, W_OK) != 0) {
    mkdir(my_cache_dir, 0755);
  }
  if(access(my_cache_dir, W_OK) == 0) {
    return;
  }
  {
    char *e = getenv("HOME");
    if(e == NULL) {
      fprintf(stderr, "error: cache directory is not available.\n       please set HOME environmental variable.");
      exit(2);
    }
    strcpy(my_cache_dir, e);
    mkdir(my_cache_dir, 0755);
  }
  if(access(my_cache_dir, W_OK) != 0) {
    fprintf(stderr, "error: cache directory '%s' is not writable,\n       or it doesn't exist.", my_cache_dir);
    exit(2);
  }
}

void exec_executable(int real_argc, char *argv[])
{
  int i;
  char **p;
  p = (char **)malloc(sizeof(char *) * (real_argc + 2)); // + 2 for executable_file_name & NULL
  p[0] = executable_path;
  for(i = 0; i < real_argc; i++) p[i + 1] = argv[i];
  p[i + 1] = NULL;
  execvp(executable_path, p); 
  fprintf(stderr, "error: failed to exec '%s'\n", executable_path);
  // we don't care memory leak here, but won't cause any problem in this case...
}

void output_skelton(char *original_script)
{
  FILE *fp;
  if(access(original_script, F_OK) == 0) {
    fprintf(stderr, "%s already exists. abort for safty.\n", original_script);
    exit(1);
  }
  if((fp = fopen(original_script, "w")) == NULL) {
    fprintf(stderr, "could not open %s\n", original_script);
    exit(2);
  }
  {
    ssize_t t;
    char *buf = (char *)malloc(PATH_MAX + 1);
    t = readlink("/proc/self/exe", buf, PATH_MAX);
    if(t == (ssize_t)-1) {
      fprintf(stderr, "failed to obtain self path. this is unlikely to occur.\n");
      exit(3);
    }
    buf[t] = '\0'; // readlink does not put '\0' at the end
    fprintf(fp, "#!%s\n", buf);
    free(buf);
  }
  if(flag_emacs_var) {
    fprintf(fp, "// -*- mode:C++; c-basic-offset:2; tab-width:2 -*-\n");
  }
  fprintf(fp, "#include <iostream>\n");
  fprintf(fp, "#include <fstream>\n");
  fprintf(fp, "#include <string>\n");
  fprintf(fp, "#include <vector>\n");
  if(flag_getopt) {
    fprintf(fp, "#include <getopt.h>\n");
  }
  fprintf(fp, "using namespace std;\n");
  fprintf(fp, "int main(int argc, char *argv[]) {\n");
  if(flag_getopt) {
    fprintf(fp, "\tint c;\n"
	  "\twhile(1) {\n"
	  "\t\tstatic struct option long_options[] = {\n"
	  "\t\t\t//{\"longflag\", no_argument      , &int_flag, 1   /*value to set*/}, // note that int_flag must be static\n"
	  "\t\t\t//{\"sflag\",    no_argument      ,         0, 's' /*equiv. short flag*/},\n"
	  "\t\t\t//{\"opt\",      optional_argument,         0, 'o' /*equiv. short flag*/},\n"
	  "\t\t\t//{\"file\",     required_argument,         0, 'f' /*equiv. short flag*/},\n"
	  "\t\t\t{0, 0, 0, 0} // end of long options\n"
	  "\t\t};\n"
	  "\t\tint option_index = 0;\n"
	  "\t\tc = getopt_long(argc, argv, \"\", long_options, &option_index);\n"
	  "\t\tif(c == -1) break;\n"
	  "\t\tswitch(c) {\n"
	  "\t\tcase 0:\n"
	  "\t\t\t// you can see long_options[option_index].name/flag and optarg (null if no argument).\n"
	  "\t\t\tbreak;\n"
	  "\t\tcase 's':\n"
	  "\t\t\t// you can see short options here. optarg is an argument.\n"
	  "\t\t\tbreak;\n"
	  "\t\t}\n"
	  "\t}\n"
	  "\tif(argc < optind + 0 /* # of non-option arguments */) {\n"
	  "\t\tfprintf(stderr, \"usage: %s [options] \\n\");\n"
	  "\t\treturn 1;\n"
	  "\t}\n"
	  "\t// here, you can reference non-option arguments by argv[optind .. argc-1]\n"
	  , original_script
	  );
  }
  fprintf(fp, "}\n");
  if(flag_perldoc) {
    fprintf(fp, "\n/*\n"
	    "=pod\n\n"
	    "=head1 NAME\n\n"
	    "%s - here comes the brief explanation\n\n"
	    "=head1 SYNOPSIS\n\n"
	    "\t [options]\n\n"
	    "=head1 DESCRIPTION\n\n"
	    "=cut\n"
	    "*/\n"
	    );
  }
  fclose(fp);
  chmod(original_script, 0755);
}

int main(int argc, char *argv[])
{
  char *original_script;
  int optind;
  optind = 1;
  while(optind < argc) {
    char *optstr;
    int p = 0;
    if(argv[optind][p] != '-') break;
    p++;
    if(argv[optind][p] == '-') p++;
    optstr = argv[optind] + p;
    if(strcmp(optstr, "skel") == 0) {
      flag_skelton = 1;
    } else if(strcmp(optstr, "getopt") == 0) {
      flag_getopt = 1;
    } else if(strcmp(optstr, "doc") == 0) {
      flag_perldoc = 1;
    } else if(strcmp(optstr, "emacs") == 0) {
      flag_emacs_var = 1;
    } else if(strcmp(optstr, "fskel") == 0) {
      flag_skelton = 1;
      flag_getopt = 1;
      flag_perldoc = 1;
      flag_emacs_var = 1;
    } else {
      fprintf(stderr, "unknown option %s\n", argv[optind]);
    }
    optind++;
  }
  if(argc < optind + 1) {
    fprintf(stderr, "usage: ilcp [options] <script>\n");
    fprintf(stderr, "       ilcp --skel [--getopt] [--doc] [--emacs] <new script>\n");
    fprintf(stderr, "       ilcp --fskel <new script>\n");
    return 1;
  }
  original_script = argv[optind];
  if(flag_skelton) {
    output_skelton(original_script);
    return 0;
  }
  generate_executable_file_name(original_script);
  check_for_cache_dir(original_script);
  generate_cache_file_names();
  if(need_to_compile(original_script)) {
    convert_script(original_script);
    compile_script();
  }
  exec_executable(argc - 2, argv + 2);
}
