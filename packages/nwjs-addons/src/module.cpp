#include "addon_api.h"

// Forward declarations for sub-module init functions
void InitClipboard(ADDON_INIT_PARAMS);
void InitFolderDialog(ADDON_INIT_PARAMS);
void InitIPC(ADDON_INIT_PARAMS);
void InitCallDLL(ADDON_INIT_PARAMS);
void InitTinyCC(ADDON_INIT_PARAMS);
void InitSQLite3(ADDON_INIT_PARAMS);
void InitCsvParser(ADDON_INIT_PARAMS);
void InitRssParser(ADDON_INIT_PARAMS);
void InitSdl2Input(ADDON_INIT_PARAMS);

ADDON_MODULE_INIT(InitAll) {
  ADDON_CALL_SUB_INIT(InitClipboard);
  ADDON_CALL_SUB_INIT(InitFolderDialog);
  ADDON_CALL_SUB_INIT(InitIPC);
  ADDON_CALL_SUB_INIT(InitCallDLL);
  ADDON_CALL_SUB_INIT(InitTinyCC);
  ADDON_CALL_SUB_INIT(InitSQLite3);
  ADDON_CALL_SUB_INIT(InitCsvParser);
  ADDON_CALL_SUB_INIT(InitRssParser);
  ADDON_CALL_SUB_INIT(InitSdl2Input);
}

ADDON_MODULE(nwjs_addons, InitAll)
