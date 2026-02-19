#include "addon_api.h"
#include "folder_dialog.h"

using namespace folder_dialog;

// Helper to parse FileOptions from JS object
static FileOptions parseFileOptions(ADDON_OBJECT_TYPE optObj) {
  FileOptions opts;
  opts.multiSelect = false;

  if (ADDON_HAS(optObj, "title")) {
    ADDON_VALUE val = ADDON_GET(optObj, "title");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      opts.title = std::string(ADDON_UTF8_VALUE(str));
    }
  }

  if (ADDON_HAS(optObj, "initialPath")) {
    ADDON_VALUE val = ADDON_GET(optObj, "initialPath");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      opts.initialPath = std::string(ADDON_UTF8_VALUE(str));
    }
  }

  if (ADDON_HAS(optObj, "defaultName")) {
    ADDON_VALUE val = ADDON_GET(optObj, "defaultName");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      opts.defaultName = std::string(ADDON_UTF8_VALUE(str));
    }
  }

  if (ADDON_HAS(optObj, "filters")) {
    ADDON_VALUE val = ADDON_GET(optObj, "filters");
    if (ADDON_IS_ARRAY(val)) {
      ADDON_ARRAY_TYPE arr = ADDON_CAST_ARRAY(val);
      for (uint32_t i = 0; i < ADDON_LENGTH(arr); i++) {
        ADDON_VALUE item = ADDON_GET_INDEX(arr, i);
        if (ADDON_IS_STRING(item)) {
          ADDON_UTF8(str, item);
          opts.filters.push_back(std::string(ADDON_UTF8_VALUE(str)));
        }
      }
    }
  }

  if (ADDON_HAS(optObj, "multiSelect")) {
    ADDON_VALUE val = ADDON_GET(optObj, "multiSelect");
    if (ADDON_IS_BOOLEAN(val)) {
      opts.multiSelect = ADDON_TO_BOOL(val);
    }
  }

  return opts;
}

ADDON_METHOD(Open) {
  ADDON_ENV;
  Options opts;

  // Parse options object if provided
  if (ADDON_ARG_COUNT() > 0 && ADDON_IS_OBJECT(ADDON_ARG(0))) {
    ADDON_OBJECT_TYPE optObj = ADDON_TO_OBJECT(ADDON_ARG(0));

    if (ADDON_HAS(optObj, "title")) {
      ADDON_VALUE titleVal = ADDON_GET(optObj, "title");
      if (ADDON_IS_STRING(titleVal)) {
        ADDON_UTF8(title, titleVal);
        opts.title = std::string(ADDON_UTF8_VALUE(title));
      }
    }

    if (ADDON_HAS(optObj, "initialPath")) {
      ADDON_VALUE pathVal = ADDON_GET(optObj, "initialPath");
      if (ADDON_IS_STRING(pathVal)) {
        ADDON_UTF8(path, pathVal);
        opts.initialPath = std::string(ADDON_UTF8_VALUE(path));
      }
    }
  }

  std::string result = open(opts);

  if (result.empty()) {
    ADDON_RETURN_NULL();
  }
  ADDON_RETURN(ADDON_STRING(result));
}

ADDON_METHOD(OpenFile) {
  ADDON_ENV;
  FileOptions opts;

  if (ADDON_ARG_COUNT() > 0 && ADDON_IS_OBJECT(ADDON_ARG(0))) {
    ADDON_OBJECT_TYPE optObj = ADDON_TO_OBJECT(ADDON_ARG(0));
    opts = parseFileOptions(optObj);
  }

  std::vector<std::string> results = openFile(opts);

  if (results.empty()) {
    ADDON_RETURN_NULL();
  }

  if (results.size() == 1 && !opts.multiSelect) {
    // Return single string for single file selection
    ADDON_RETURN(ADDON_STRING(results[0]));
  }

  // Return array for multiselect
  ADDON_ARRAY_TYPE arr = ADDON_ARRAY(results.size());
  for (size_t i = 0; i < results.size(); i++) {
    ADDON_SET_INDEX(arr, i, ADDON_STRING(results[i]));
  }
  ADDON_RETURN(arr);
}

ADDON_METHOD(SaveFile) {
  ADDON_ENV;
  FileOptions opts;

  if (ADDON_ARG_COUNT() > 0 && ADDON_IS_OBJECT(ADDON_ARG(0))) {
    ADDON_OBJECT_TYPE optObj = ADDON_TO_OBJECT(ADDON_ARG(0));
    opts = parseFileOptions(optObj);
  }

  std::string result = saveFile(opts);

  if (result.empty()) {
    ADDON_RETURN_NULL();
  }
  ADDON_RETURN(ADDON_STRING(result));
}

void InitFolderDialog(ADDON_INIT_PARAMS) {
  ADDON_EXPORT_FUNCTION(exports, "folderDialogOpen", Open);
  ADDON_EXPORT_FUNCTION(exports, "fileDialogOpen", OpenFile);
  ADDON_EXPORT_FUNCTION(exports, "fileDialogSave", SaveFile);
}
