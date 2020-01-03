#include <iron/full.h>
#include <icydb.h>
#include "string_intern.h"
#include "u32_to_u32.h"
#include "string_table.h"
#include "u64_table.h"
u64_table * loaded_modules;
#include<dlfcn.h>
#include "tests.h"

int load_module(const char * name){

  u64 moduleid = intern_string(name);
  
  if(u64_table_try_get(loaded_modules, &moduleid, NULL)){
    logd("Module already %s loaded\n", name);
    return moduleid;
  }

  void * module = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
  if(module == NULL){
    logd("Unable to load module %s\n", name);
    return -1;
  }

  u64_table_set(loaded_modules, moduleid, 0);

  void (* init_module)() = dlsym(module, "init_module");
  init_module();
  return 0;
}

int main(){
  logd_enable = true;
  loaded_modules = u64_table_create(NULL);
  load_module("./graphics.so");
  load_module("./tests.so");
  return 0;
}
