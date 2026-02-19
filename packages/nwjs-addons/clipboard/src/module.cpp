#include "addon_api.h"
#include "clipboard.h"

using namespace clipboard;

ADDON_METHOD(GetType) {
  ADDON_ENV;
  ClipboardType type = getType();

  const char* typeStr;
  switch (type) {
    case CLIPBOARD_EMPTY:   typeStr = "empty"; break;
    case CLIPBOARD_TEXT:    typeStr = "text"; break;
    case CLIPBOARD_FILES:   typeStr = "files"; break;
    case CLIPBOARD_IMAGE:   typeStr = "image"; break;
    default:                typeStr = "unknown"; break;
  }

  ADDON_RETURN(ADDON_STRING(typeStr));
}

ADDON_METHOD(HasText) {
  ADDON_ENV;
  ADDON_RETURN(ADDON_BOOL(hasText()));
}

ADDON_METHOD(HasFiles) {
  ADDON_ENV;
  ADDON_RETURN(ADDON_BOOL(hasFiles()));
}

ADDON_METHOD(HasImage) {
  ADDON_ENV;
  ADDON_RETURN(ADDON_BOOL(hasImage()));
}

ADDON_METHOD(GetText) {
  ADDON_ENV;
  std::string text = getText();
  if (text.empty()) {
    ADDON_RETURN_NULL();
  }
  ADDON_RETURN(ADDON_STRING(text));
}

ADDON_METHOD(CopyText) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(text, ADDON_ARG(0));
  bool success = copyText(std::string(ADDON_UTF8_VALUE(text)));
  ADDON_RETURN(ADDON_BOOL(success));
}

ADDON_METHOD(GetFiles) {
  ADDON_ENV;
  std::vector<std::string> files = getFiles();

  ADDON_ARRAY_TYPE result = ADDON_ARRAY(files.size());

  for (size_t i = 0; i < files.size(); i++) {
    ADDON_SET_INDEX(result, i, ADDON_STRING(files[i]));
  }

  ADDON_RETURN(result);
}

ADDON_METHOD(CopyFiles) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_ARRAY(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be an array of strings");
    ADDON_VOID_RETURN();
  }

  ADDON_ARRAY_TYPE arr = ADDON_CAST_ARRAY(ADDON_ARG(0));
  std::vector<std::string> paths;

  for (uint32_t i = 0; i < ADDON_LENGTH(arr); i++) {
    ADDON_VALUE val = ADDON_GET_INDEX(arr, i);
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      paths.push_back(std::string(ADDON_UTF8_VALUE(str)));
    }
  }

  bool success = copyFiles(paths);
  ADDON_RETURN(ADDON_BOOL(success));
}

ADDON_METHOD(CutFiles) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_ARRAY(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be an array of strings");
    ADDON_VOID_RETURN();
  }

  ADDON_ARRAY_TYPE arr = ADDON_CAST_ARRAY(ADDON_ARG(0));
  std::vector<std::string> paths;

  for (uint32_t i = 0; i < ADDON_LENGTH(arr); i++) {
    ADDON_VALUE val = ADDON_GET_INDEX(arr, i);
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      paths.push_back(std::string(ADDON_UTF8_VALUE(str)));
    }
  }

  bool success = cutFiles(paths);
  ADDON_RETURN(ADDON_BOOL(success));
}

ADDON_METHOD(PasteFiles) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a string (destination directory)");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(destDir, ADDON_ARG(0));
  std::vector<std::string> newPaths = pasteFiles(std::string(ADDON_UTF8_VALUE(destDir)));

  ADDON_ARRAY_TYPE result = ADDON_ARRAY(newPaths.size());

  for (size_t i = 0; i < newPaths.size(); i++) {
    ADDON_SET_INDEX(result, i, ADDON_STRING(newPaths[i]));
  }

  ADDON_RETURN(result);
}

ADDON_METHOD(IsCutOperation) {
  ADDON_ENV;
  ADDON_RETURN(ADDON_BOOL(isCutOperation()));
}

ADDON_METHOD(GetImageSize) {
  ADDON_ENV;
  int width = 0;
  int height = 0;

  if (getImageSize(width, height)) {
    ADDON_OBJECT_TYPE result = ADDON_OBJECT();
    ADDON_SET(result, "width", ADDON_INT(width));
    ADDON_SET(result, "height", ADDON_INT(height));
    ADDON_RETURN(result);
  }
  ADDON_RETURN_NULL();
}

ADDON_METHOD(SaveImageToFile) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a string (file path)");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(filePath, ADDON_ARG(0));
  bool success = saveImageToFile(std::string(ADDON_UTF8_VALUE(filePath)));
  ADDON_RETURN(ADDON_BOOL(success));
}

ADDON_METHOD(Clear) {
  ADDON_ENV;
  bool success = clear();
  ADDON_RETURN(ADDON_BOOL(success));
}

void InitClipboard(ADDON_INIT_PARAMS) {
  ADDON_EXPORT_FUNCTION(exports, "clipboardGetType", GetType);
  ADDON_EXPORT_FUNCTION(exports, "clipboardHasText", HasText);
  ADDON_EXPORT_FUNCTION(exports, "clipboardHasFiles", HasFiles);
  ADDON_EXPORT_FUNCTION(exports, "clipboardHasImage", HasImage);
  ADDON_EXPORT_FUNCTION(exports, "clipboardGetText", GetText);
  ADDON_EXPORT_FUNCTION(exports, "clipboardCopyText", CopyText);
  ADDON_EXPORT_FUNCTION(exports, "clipboardGetFiles", GetFiles);
  ADDON_EXPORT_FUNCTION(exports, "clipboardCopyFiles", CopyFiles);
  ADDON_EXPORT_FUNCTION(exports, "clipboardCutFiles", CutFiles);
  ADDON_EXPORT_FUNCTION(exports, "clipboardPasteFiles", PasteFiles);
  ADDON_EXPORT_FUNCTION(exports, "clipboardIsCutOperation", IsCutOperation);
  ADDON_EXPORT_FUNCTION(exports, "clipboardGetImageSize", GetImageSize);
  ADDON_EXPORT_FUNCTION(exports, "clipboardSaveImageToFile", SaveImageToFile);
  ADDON_EXPORT_FUNCTION(exports, "clipboardClear", Clear);
}
