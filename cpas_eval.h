#ifndef _EVAL_HEADER
#define _EVAL_HEADER

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>

extern const char* _cpas_compile_option;
extern const char* _cpas_compile_option_last;
extern const char* _cpas_shared_library_base_name;
extern const char* _cpas_include_directives;
extern void *pthread_getspecific(pthread_key_t key) __attribute__ ((weak));

class Eval{
  std::vector<std::string> file_names_to_be_deleted_on_exit;
  std::vector<void*> shared_library_handles_to_be_dlclosed_on_exit;
  void destroy() {
    for(size_t i = 0; i < shared_library_handles_to_be_dlclosed_on_exit.size(); i++) {
      dlclose(shared_library_handles_to_be_dlclosed_on_exit[i]);
    }
    shared_library_handles_to_be_dlclosed_on_exit.clear();
    for(size_t i = 0; i < file_names_to_be_deleted_on_exit.size(); i++) {
      if(is_debugging) std::cerr << "unlink " << file_names_to_be_deleted_on_exit[i] << std::endl;
      unlink(file_names_to_be_deleted_on_exit[i].c_str());
    }
    file_names_to_be_deleted_on_exit.clear();
  }
  pthread_t threadID;
  std::string lastErrorMessage;
  bool is_debugging;

 public:
  struct Handle {
    void* handle;
    Handle(void* handle) : handle(handle){}
    Handle() {}
    template<class FUNC_TYPE>
    void get_function(const char* function_name, FUNC_TYPE** user_function) {
      *user_function = (FUNC_TYPE*)dlsym(handle, function_name);
    }
  };
  Eval() {
    if(pthread_getspecific) {
      threadID = pthread_self();
    } else {
      // If single-threaded,
      threadID = 0;
    }
    is_debugging = getenv("CPAS_DEBUG") != NULL;
  }
  virtual ~Eval() {
    destroy();
  }
  const std::string getLastError() {
    return lastErrorMessage;
  }
  Handle eval_function(const char* function_string) {
    std::string shared_library_file_name = _cpas_shared_library_base_name;
    std::string shared_library_source_file_name = _cpas_shared_library_base_name;
    {
      char buf[128];
      std::sprintf(buf, ".%p_%d.so", this, threadID);
      shared_library_file_name += buf;
      std::sprintf(buf, ".%p_%d.cpp", this, threadID);
      shared_library_source_file_name += buf;
    }
    std::FILE* fp = std::fopen(shared_library_source_file_name.c_str(), "w");
    if(fp == NULL) {
      lastErrorMessage = std::string("Cannot open temporary source file '") + shared_library_source_file_name + "'";
      return NULL;
    }
    std::fprintf(fp, "%s\n", _cpas_include_directives);
    std::fprintf(fp, "%s\n", function_string);
    std::fclose(fp);
    file_names_to_be_deleted_on_exit.push_back(shared_library_source_file_name);
    {
      int status;
      pid_t child;
      if((child = fork()) == 0) {
	char buf[2048];
	int mystatus;
	std::snprintf(buf, sizeof(buf), "g++ -shared -fPIC %s -o %s %s",
		      shared_library_source_file_name.c_str(),
		      shared_library_file_name.c_str(),
		      _cpas_compile_option);
	if(is_debugging) std::cerr << buf << std::endl;
	mystatus = std::system(buf);
	if(mystatus == -1) {
	  std::abort();
	}
	std::exit(WEXITSTATUS(mystatus));
      }
      wait(&status);
      if(!WIFEXITED(status) || (WIFEXITED(status) && WEXITSTATUS(status) != 0)) {
	lastErrorMessage = "Compile error (source filename:";
	lastErrorMessage += shared_library_source_file_name;
	lastErrorMessage += ")";
	return NULL;
      }
      file_names_to_be_deleted_on_exit.push_back(shared_library_file_name);
    }    
    void *so_handle = dlopen(shared_library_file_name.c_str(), RTLD_LAZY);
    if(so_handle == NULL) {
      lastErrorMessage = "dlopen error (filename:'";
      lastErrorMessage += shared_library_file_name;
      lastErrorMessage += ")";
      return NULL;
    }
    shared_library_handles_to_be_dlclosed_on_exit.push_back(so_handle);
    lastErrorMessage = "no error";
    return so_handle;
  }
};

#endif // #ifndef _EVAL_HEADER

