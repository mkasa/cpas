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

#ifndef PREFIX_DIR
static char *prefix_dir = "/usr";
#else
static char *prefix_dir = PREFIX_DIR;
#endif

static char *cache_dir_name = ".cpas";

char executable_file_name[PATH_MAX];
char my_cache_dir[PATH_MAX + 10];
char source_code_path[PATH_MAX + 18];
char executable_path[PATH_MAX + 18];
char compiler_name[PATH_MAX + 18] = "g++";
char compile_options[4096];
char compile_options_last[4096];
static int flag_skelton    = 0;
static int flag_getopt     = 0;
static int flag_perldoc    = 0;
static int flag_stackdump  = 0;
static int flag_debugmacro = 0;
static int flag_emacs_var  = 0;
static int flag_icpc       = 0; /* Use Intel C++ when outputting a skel*/
 
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
  if(sbuf.st_mtime > ebuf.st_mtime)
    return 1; // it's old! 
  FILE *ifp;
  char buf[1024];
  if((ifp = fopen(original_script, "r")) == NULL) {
    fprintf(stderr, "error: cannot open script '%s'\n", original_script);
    exit(5);
  }
  int has_to_check_dependencies_strictly = 0;
  while(!feof(ifp)) {
    char *l = fgets(buf, sizeof(buf), ifp);
    if(l == NULL) break;
    if(buf[0] == '/' && buf[1] == '/') {
      char *p;
      p = strstr(buf + 2, "depend:");
      if(p != NULL) {
	has_to_check_dependencies_strictly = 1;
      }
    }    
  }
  if(has_to_check_dependencies_strictly) {
    rewind(ifp);
    // NOTE: This feature is not implemented yet.
    // TODO: Check dependencies strictly here.
  }
  fclose(ifp);
  return 0;
}

void ensure_a_string_ends_with_space(char* s, size_t buffer_length)
{
  const int l = strlen(s);
  if(0 < l && s[l - 1] != ' ') {
    if(l + 1 < buffer_length) {
      s[l] = ' ';
      s[l + 1] = '\0';
    }
  }
}

void chomp(char* s)
{
  char *p = s;
  while(*p != '\0') p++;
  p--;
  if(s < p && *p == '\n') *p-- = '\0';
  if(s < p && *p == '\r') *p-- = '\0';
  if(s < p && *p == '\n') *p-- = '\0';
}

void convert_script(char *original_script)
{
  FILE *ifp;
  FILE *ofp;
  char buf[1024];
  if((ifp = fopen(original_script, "r")) == NULL) {
    fprintf(stderr, "error: cannot open script '%s'\n", original_script);
    exit(3);
  }
  if((ofp = fopen(source_code_path, "w")) == NULL) {
    fprintf(stderr, "error: cannot open output cache script '%s'\n", source_code_path);
    exit(4);
  }
  int has_included_eval_header = 0;
  while(!feof(ifp)) {
    char *l = fgets(buf, sizeof(buf), ifp);
    if(l == NULL) break;
    if(buf[0] == '#' && buf[1] == '!') {
      fprintf(ofp, "//%s\n", buf); // for not to change the number of lines.
      continue;
    }
    chomp(buf);
    char *nsp = buf;
    while(nsp < buf + sizeof(buf) && (*nsp == ' ' || *nsp == '\t')) nsp++;
    if(*nsp == '#') {
      char *p = nsp + 1;
      while(p < buf + sizeof(buf) && (*p == ' ' || *p == '\t')) p++;
      p = strstr(p, "include");
      if(p != NULL) {
	// NOTE: Should use a better parser here...
	//       This parser will not catch '#include< eval.h >', for example.
	if(strstr(p, "\"eval.h\"") != NULL) has_included_eval_header = 1;
	if(strstr(p, "<eval.h>") != NULL) has_included_eval_header = 1;
      }
    }
    if(nsp + 2 < buf + sizeof(buf) && nsp[0] == '/' && nsp[1] == '/') {
      char *p = nsp + 2;
      while(p < buf + sizeof(buf) && (*p == ' ' || *p == '\t')) p++;
      char *bep = strstr(p, "opt:");
      if(bep != NULL) {
	ensure_a_string_ends_with_space(compile_options, sizeof(compile_options));
	strncat(compile_options, bep + 4, sizeof(compile_options));
      }
      char *aep = strstr(p, "opta:");
      if(aep != NULL) {
	ensure_a_string_ends_with_space(compile_options_last, sizeof(compile_options_last));
	strncat(compile_options_last, aep + 5, sizeof(compile_options_last));
      }
      char *cep = strstr(p, "compiler:");
      if(cep != NULL) {
        strncpy(compiler_name, cep + 9, sizeof(compiler_name));
      }
      continue;
    }
    fprintf(ofp, "%s\n", l);
  }
  if(has_included_eval_header) {
    ensure_a_string_ends_with_space(compile_options, sizeof(compile_options));
    strncat(compile_options, "-rdynamic", sizeof(compile_options));
    ensure_a_string_ends_with_space(compile_options_last, sizeof(compile_options_last));
    strncat(compile_options_last, "-ldl", sizeof(compile_options_last));
    fprintf(ofp, "\n// cpas generated code for eval\n");
    fprintf(ofp, "const char* _cpas_compile_option = \"%s\";\n", compile_options);
    fprintf(ofp, "const char* _cpas_compile_option_last = \"%s\";\n", compile_options_last);
    fprintf(ofp, "const char* _cpas_shared_library_base_name = \"%s_so\";\n", executable_path);
    fprintf(ofp, "const char* _cpas_include_directives = ""\n");
    rewind(ifp);
    while(!feof(ifp)) {
      char *l = fgets(buf, sizeof(buf), ifp);
      if(l == NULL) break;
      if(buf[0] == '#' && buf[1] == '!') continue;
      chomp(buf);
      char *nsp = buf;
      while(nsp < buf + sizeof(buf) && (*nsp == ' ' || *nsp == '\t')) nsp++;
      if(*nsp == '#') {
	fprintf(ofp, "\t\"%s\\n\"\n", buf);
      }
    }
    fprintf(ofp, "\t;\n", buf);
  }
  fclose(ofp);
  fclose(ifp);
}

void compile_script(char *script_path)
{
  int status;
  pid_t child;
  if((child = fork()) == 0) {
    char canonical_path[PATH_MAX];
    char buf[2048];
    int mystatus;
    char *original_code_real_path;
    original_code_real_path = realpath(script_path, canonical_path);
    if(original_code_real_path == NULL) {
      fprintf(stderr, "error: could not resolve the real path of '%s'\n", script_path);
      exit(6);
    }
    {
      int p = strlen(original_code_real_path);
      while(0 < p && canonical_path[p - 1] != '/') p--;
      if(0 < p) canonical_path[p] = '\0';
    }
    snprintf(buf, sizeof(buf), "%s %s %s -o %s -I%s %s\n", compiler_name, compile_options, source_code_path, executable_path, canonical_path, compile_options_last);
    if(getenv("CPAS_DEBUG") != NULL)
      fprintf(stderr, buf);
    mystatus = system(buf);
    if(mystatus == -1) abort();
    exit(WEXITSTATUS(mystatus));
  }
  wait(&status);
  if(!WIFEXITED(status) || (WIFEXITED(status) && WEXITSTATUS(status) != 0)) {
    exit(6);
  }
}

void remove_exe()
{
  unlink(executable_path);
}

void generate_executable_file_name(char *script_path)
{
  const int CRCF = 0x04C11DB7;
  const int LEFT = 0x04000000;
  int       crc  =  0x1234567;
  char canonical_path[PATH_MAX];
  char *p;
  int i;
  char *r;
  r = realpath(script_path, canonical_path);
  if(r == NULL) {
    fprintf(stderr, "error: cannot find '%s'\n", script_path);
    exit(5);
  }
  for(p = canonical_path; *p; p++) {
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
      fprintf(stderr, "failed to obtain self path. this is unlikely to occur if you are using Linux. Other OS is not supported yet.\n");
      exit(3);
    }
    buf[t] = '\0'; // readlink does not put '\0' at the end
    fprintf(fp, "#!%s\n", buf);
    free(buf);
  }
  if(flag_emacs_var) {
    fprintf(fp, "// -*- mode:C++; c-basic-offset:2; tab-width:2 -*-\n");
  }
  fprintf(fp, "//opt: -g -O1");
  if(flag_stackdump) {
    fprintf(fp, " -I%s/include", prefix_dir);
  }
  fprintf(fp, "\n");
  if(flag_icpc) {
    fprintf(fp, "//compiler: icpc\n");
  }
  fprintf(fp, "#include <iostream>\n");
  fprintf(fp, "#include <fstream>\n");
  fprintf(fp, "#include <string>\n");
  fprintf(fp, "#include <vector>\n");
  if(flag_getopt) {
    fprintf(fp, "#include <getopt.h>\n");
  }
  if(flag_stackdump) {
    fprintf(fp, "#include <stackdump.h>\n");
  }
  if(flag_debugmacro) {
    fprintf(fp, "#include <debug.h>\n");
  }
  fprintf(fp, "\nusing namespace std;\n\n");
  fprintf(fp, "int main(int argc, char *argv[]) {\n");
  if(flag_stackdump) {
    fprintf(fp, "  GDB_On_SEGV gos(argv[0]);\n");
  }
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
    char *base_name, *p;
    base_name = original_script;
    while((p = strchr(base_name, '/')) != NULL)
      base_name = p + 1;
    fprintf(fp, "\n/*\n"
	    "=pod\n\n"
	    "=head1 NAME\n\n"
	    "%s - here comes the brief explanation\n\n"
	    "=head1 SYNOPSIS\n\n"
	    "\t [options]\n\n"
	    "=head1 DESCRIPTION\n\n"
	    "=cut\n"
	    "*/\n",
            base_name
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
    } else if(strcmp(optstr, "sdump") == 0) {
      flag_stackdump = 1;
    } else if(strcmp(optstr, "dmacro") == 0) {
      flag_debugmacro = 1;
    } else if(strcmp(optstr, "icpc") == 0) {
      flag_icpc = 1;
    } else if(strcmp(optstr, "fskel") == 0) {
      flag_skelton = 1;
      flag_getopt = 1;
      flag_perldoc = 1;
      flag_emacs_var = 1;
      flag_stackdump = 1;
      flag_icpc = 1;
      flag_debugmacro = 1;
    } else {
      fprintf(stderr, "unknown option %s\n", argv[optind]);
    }
    optind++;
  }
  if(argc < optind + 1) {
    fprintf(stderr, "usage: cpas [options] <script>\n");
    fprintf(stderr, "       cpas --skel [--getopt] [--doc] [--emacs] [--sdump] [--dmacro] [--icpc] <new script>\n");
    fprintf(stderr, "       cpas --fskel <new script>\n");
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
    remove_exe();
    compile_script(original_script);
  }
  exec_executable(argc - 2, argv + 2);
}
