#include "addon_api.h"
#include <fstream>
#include "csv_parser.h"

using namespace csvparser;

static ParseOptions extractParseOptions(ADDON_OBJECT_TYPE optObj) {
  ParseOptions opts;

  if (ADDON_HAS(optObj, "delimiter")) {
    ADDON_VALUE val = ADDON_GET(optObj, "delimiter");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      if (ADDON_UTF8_LENGTH(str) > 0) {
        opts.delimiter = ADDON_UTF8_VALUE(str)[0];
      }
    }
  }

  if (ADDON_HAS(optObj, "quote")) {
    ADDON_VALUE val = ADDON_GET(optObj, "quote");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      if (ADDON_UTF8_LENGTH(str) > 0) {
        opts.quote = ADDON_UTF8_VALUE(str)[0];
      }
    }
  }

  if (ADDON_HAS(optObj, "escape")) {
    ADDON_VALUE val = ADDON_GET(optObj, "escape");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      if (ADDON_UTF8_LENGTH(str) > 0) {
        opts.escape = ADDON_UTF8_VALUE(str)[0];
      }
    }
  }

  if (ADDON_HAS(optObj, "skipEmptyLines")) {
    ADDON_VALUE val = ADDON_GET(optObj, "skipEmptyLines");
    if (ADDON_IS_BOOLEAN(val)) {
      opts.skipEmptyLines = ADDON_BOOL_VALUE(val);
    }
  }

  if (ADDON_HAS(optObj, "trim")) {
    ADDON_VALUE val = ADDON_GET(optObj, "trim");
    if (ADDON_IS_BOOLEAN(val)) {
      opts.trim = ADDON_BOOL_VALUE(val);
    }
  }

  return opts;
}

static StringifyOptions extractStringifyOptions(ADDON_OBJECT_TYPE optObj) {
  StringifyOptions opts;

  if (ADDON_HAS(optObj, "delimiter")) {
    ADDON_VALUE val = ADDON_GET(optObj, "delimiter");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      if (ADDON_UTF8_LENGTH(str) > 0) {
        opts.delimiter = ADDON_UTF8_VALUE(str)[0];
      }
    }
  }

  if (ADDON_HAS(optObj, "quote")) {
    ADDON_VALUE val = ADDON_GET(optObj, "quote");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      if (ADDON_UTF8_LENGTH(str) > 0) {
        opts.quote = ADDON_UTF8_VALUE(str)[0];
      }
    }
  }

  if (ADDON_HAS(optObj, "quoteAll")) {
    ADDON_VALUE val = ADDON_GET(optObj, "quoteAll");
    if (ADDON_IS_BOOLEAN(val)) {
      opts.quoteAll = ADDON_BOOL_VALUE(val);
    }
  }

  if (ADDON_HAS(optObj, "lineEnding")) {
    ADDON_VALUE val = ADDON_GET(optObj, "lineEnding");
    if (ADDON_IS_STRING(val)) {
      ADDON_UTF8(str, val);
      opts.lineEnding = std::string(ADDON_UTF8_VALUE(str));
    }
  }

  return opts;
}

static ADDON_ARRAY_TYPE rowsToJsArray(const std::vector<std::vector<std::string>>& rows) {
  ADDON_ARRAY_TYPE result = ADDON_ARRAY(rows.size());

  for (size_t i = 0; i < rows.size(); i++) {
    const std::vector<std::string>& row = rows[i];
    ADDON_ARRAY_TYPE jsRow = ADDON_ARRAY(row.size());

    for (size_t j = 0; j < row.size(); j++) {
      ADDON_SET_INDEX(jsRow, j, ADDON_STRING(row[j]));
    }

    ADDON_SET_INDEX(result, i, jsRow);
  }

  return result;
}

static std::vector<std::vector<std::string>> jsArrayToRows(ADDON_ARRAY_TYPE arr) {
  std::vector<std::vector<std::string>> rows;

  for (uint32_t i = 0; i < ADDON_LENGTH(arr); i++) {
    ADDON_VALUE rowVal = ADDON_GET_INDEX(arr, i);
    std::vector<std::string> row;

    if (ADDON_IS_ARRAY(rowVal)) {
      ADDON_ARRAY_TYPE jsRow = ADDON_CAST_ARRAY(rowVal);
      for (uint32_t j = 0; j < ADDON_LENGTH(jsRow); j++) {
        ADDON_VALUE cellVal = ADDON_GET_INDEX(jsRow, j);
        if (ADDON_IS_STRING(cellVal)) {
          ADDON_UTF8(str, cellVal);
          row.push_back(std::string(ADDON_UTF8_VALUE(str)));
        } else {
          row.push_back("");
        }
      }
    }

    rows.push_back(row);
  }

  return rows;
}

ADDON_METHOD(Parse) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(content, ADDON_ARG(0));
  ParseOptions opts;

  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_OBJECT(ADDON_ARG(1))) {
    opts = extractParseOptions(ADDON_AS_OBJECT(ADDON_ARG(1)));
  }

  std::vector<std::vector<std::string>> rows = parse(std::string(ADDON_UTF8_VALUE(content)), opts);
  ADDON_RETURN(rowsToJsArray(rows));
}

ADDON_METHOD(ParseFile) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a file path string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(filePath, ADDON_ARG(0));
  ParseOptions opts;

  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_OBJECT(ADDON_ARG(1))) {
    opts = extractParseOptions(ADDON_AS_OBJECT(ADDON_ARG(1)));
  }

  std::string content = readFileContents(std::string(ADDON_UTF8_VALUE(filePath)));
  if (content.empty()) {
    std::string path(ADDON_UTF8_VALUE(filePath));
    std::ifstream testFile(path.c_str());
    if (!testFile.is_open()) {
      ADDON_THROW_ERROR("Could not open file");
      ADDON_VOID_RETURN();
    }
    testFile.close();
  }

  std::vector<std::vector<std::string>> rows = parse(content, opts);
  ADDON_RETURN(rowsToJsArray(rows));
}

ADDON_METHOD(Stringify) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_ARRAY(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be an array");
    ADDON_VOID_RETURN();
  }

  ADDON_ARRAY_TYPE arr = ADDON_CAST_ARRAY(ADDON_ARG(0));
  StringifyOptions opts;

  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_OBJECT(ADDON_ARG(1))) {
    opts = extractStringifyOptions(ADDON_AS_OBJECT(ADDON_ARG(1)));
  }

  std::vector<std::vector<std::string>> rows = jsArrayToRows(arr);
  std::string result = stringify(rows, opts);

  ADDON_RETURN(ADDON_STRING(result));
}

ADDON_METHOD(WriteFile) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 2 || !ADDON_IS_STRING(ADDON_ARG(0)) || !ADDON_IS_STRING(ADDON_ARG(1))) {
    ADDON_THROW_TYPE_ERROR("Arguments must be (filePath, content)");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(filePath, ADDON_ARG(0));
  ADDON_UTF8(content, ADDON_ARG(1));

  bool success = writeFile(std::string(ADDON_UTF8_VALUE(filePath)), std::string(ADDON_UTF8_VALUE(content)));
  ADDON_RETURN(ADDON_BOOL(success));
}

void InitCsvParser(ADDON_INIT_PARAMS) {
  ADDON_EXPORT_FUNCTION(exports, "csvParse", Parse);
  ADDON_EXPORT_FUNCTION(exports, "csvParseFile", ParseFile);
  ADDON_EXPORT_FUNCTION(exports, "csvStringify", Stringify);
  ADDON_EXPORT_FUNCTION(exports, "csvWriteFile", WriteFile);
}
