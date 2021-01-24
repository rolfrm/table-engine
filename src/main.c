#include <iron/full.h>
#include <icydb.h>
#include "string_intern.h"
#include "u32_to_u32.h"
#include "string_table.h"
#include "u64_table.h"
#include "string_redirect_table.h"

u64_table * loaded_modules;
string_redirect_table * modules;
string_table * module_names;

#include<dlfcn.h>
#include "tests.h"

static bool is_loaded(u64 moduleid){
  return u64_table_try_get(loaded_modules, &moduleid, NULL);
}

int load_module(const char * name){

  u64 moduleid = intern_string(name);
  
  if(u64_table_try_get(loaded_modules, &moduleid, NULL)){
    logd("Module already %s loaded\n", name);
    return moduleid;
  }

  void * module = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
  if(module == NULL){
    logd("Unable to load module %s\n%s\n", name, dlerror());
    return -1;
  }

  u64_table_set(loaded_modules, moduleid, 0);

  { // inscribe loaded module
    string_table_indexes idx;
    if(string_redirect_table_try_get(modules, &moduleid, &idx) == false){
      idx = string_table_alloc_sequence(module_names, strlen(name) + 1);
      void * dst = module_names->data + idx.index;
      memcpy(dst, name, idx.count);
      string_redirect_table_set(modules, moduleid, idx);
    }
  }
  void (* init_module)() = dlsym(module, "init_module");
  init_module();
  return 0;
}

static void init_load(){
  for(size_t i = 0; i < modules->count; i++){
    u64 key = modules->key[i + 1];
    if(!is_loaded(key)){
      string_table_indexes idx = modules->value[i + 1];
      var name = (char *) module_names->data + idx.index;
      printf("Autoloading %s\n", name);
      load_module(name);
    }
  }
}

void continue_init_load(){
  init_load();
}


void binui_test();

int main(){
  binui_test();
  
  logd_enable = true;
  loaded_modules = u64_table_create(NULL);
  modules = string_redirect_table_create("modules");
  module_names = string_table_create("module names");
  init_load();
  load_module("./graphics.so");
  load_module("./tests.so");
  return 0;
}
