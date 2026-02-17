#include <nan.h>

// Forward declarations for sub-module init functions
void InitClipboard(v8::Local<v8::Object> exports);
void InitFolderDialog(v8::Local<v8::Object> exports);
void InitIPC(v8::Local<v8::Object> exports);
void InitCallDLL(v8::Local<v8::Object> exports);
void InitTinyCC(v8::Local<v8::Object> exports);
void InitSQLite3(v8::Local<v8::Object> exports);
void InitCsvParser(v8::Local<v8::Object> exports);
void InitRssParser(v8::Local<v8::Object> exports);
void InitSdl2Input(v8::Local<v8::Object> exports);

NAN_MODULE_INIT(InitAll) {
  InitClipboard(target);
  InitFolderDialog(target);
  InitIPC(target);
  InitCallDLL(target);
  InitTinyCC(target);
  InitSQLite3(target);
  InitCsvParser(target);
  InitRssParser(target);
  InitSdl2Input(target);
}

NODE_MODULE(nwjs_addons, InitAll)
