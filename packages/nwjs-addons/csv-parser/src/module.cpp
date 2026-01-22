#include <nan.h>
#include <fstream>
#include "csv_parser.h"

using namespace v8;
using namespace csvparser;

static ParseOptions extractParseOptions(Local<Object> optObj) {
  ParseOptions opts;

  Local<String> delimKey = Nan::New("delimiter").ToLocalChecked();
  Local<String> quoteKey = Nan::New("quote").ToLocalChecked();
  Local<String> escapeKey = Nan::New("escape").ToLocalChecked();
  Local<String> skipEmptyKey = Nan::New("skipEmptyLines").ToLocalChecked();
  Local<String> trimKey = Nan::New("trim").ToLocalChecked();

  if (Nan::Has(optObj, delimKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, delimKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      if (str.length() > 0) {
        opts.delimiter = (*str)[0];
      }
    }
  }

  if (Nan::Has(optObj, quoteKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, quoteKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      if (str.length() > 0) {
        opts.quote = (*str)[0];
      }
    }
  }

  if (Nan::Has(optObj, escapeKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, escapeKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      if (str.length() > 0) {
        opts.escape = (*str)[0];
      }
    }
  }

  if (Nan::Has(optObj, skipEmptyKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, skipEmptyKey).ToLocalChecked();
    if (val->IsBoolean()) {
      opts.skipEmptyLines = val->BooleanValue();
    }
  }

  if (Nan::Has(optObj, trimKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, trimKey).ToLocalChecked();
    if (val->IsBoolean()) {
      opts.trim = val->BooleanValue();
    }
  }

  return opts;
}

static StringifyOptions extractStringifyOptions(Local<Object> optObj) {
  StringifyOptions opts;

  Local<String> delimKey = Nan::New("delimiter").ToLocalChecked();
  Local<String> quoteKey = Nan::New("quote").ToLocalChecked();
  Local<String> quoteAllKey = Nan::New("quoteAll").ToLocalChecked();
  Local<String> lineEndingKey = Nan::New("lineEnding").ToLocalChecked();

  if (Nan::Has(optObj, delimKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, delimKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      if (str.length() > 0) {
        opts.delimiter = (*str)[0];
      }
    }
  }

  if (Nan::Has(optObj, quoteKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, quoteKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      if (str.length() > 0) {
        opts.quote = (*str)[0];
      }
    }
  }

  if (Nan::Has(optObj, quoteAllKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, quoteAllKey).ToLocalChecked();
    if (val->IsBoolean()) {
      opts.quoteAll = val->BooleanValue();
    }
  }

  if (Nan::Has(optObj, lineEndingKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, lineEndingKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      opts.lineEnding = std::string(*str);
    }
  }

  return opts;
}

static Local<Array> rowsToJsArray(const std::vector<std::vector<std::string>>& rows) {
  Local<Array> result = Nan::New<Array>(static_cast<uint32_t>(rows.size()));

  for (size_t i = 0; i < rows.size(); i++) {
    const std::vector<std::string>& row = rows[i];
    Local<Array> jsRow = Nan::New<Array>(static_cast<uint32_t>(row.size()));

    for (size_t j = 0; j < row.size(); j++) {
      Nan::Set(jsRow, static_cast<uint32_t>(j), Nan::New(row[j]).ToLocalChecked());
    }

    Nan::Set(result, static_cast<uint32_t>(i), jsRow);
  }

  return result;
}

static std::vector<std::vector<std::string>> jsArrayToRows(Local<Array> arr) {
  std::vector<std::vector<std::string>> rows;

  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> rowVal = Nan::Get(arr, i).ToLocalChecked();
    std::vector<std::string> row;

    if (rowVal->IsArray()) {
      Local<Array> jsRow = Local<Array>::Cast(rowVal);
      for (uint32_t j = 0; j < jsRow->Length(); j++) {
        Local<Value> cellVal = Nan::Get(jsRow, j).ToLocalChecked();
        if (cellVal->IsString()) {
          Nan::Utf8String str(cellVal);
          row.push_back(std::string(*str));
        } else {
          row.push_back("");
        }
      }
    }

    rows.push_back(row);
  }

  return rows;
}

NAN_METHOD(Parse) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a string");
    return;
  }

  Nan::Utf8String content(info[0]);
  ParseOptions opts;

  if (info.Length() >= 2 && info[1]->IsObject()) {
    opts = extractParseOptions(Local<Object>::Cast(info[1]));
  }

  std::vector<std::vector<std::string>> rows = parse(std::string(*content), opts);
  info.GetReturnValue().Set(rowsToJsArray(rows));
}

NAN_METHOD(ParseFile) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a file path string");
    return;
  }

  Nan::Utf8String filePath(info[0]);
  ParseOptions opts;

  if (info.Length() >= 2 && info[1]->IsObject()) {
    opts = extractParseOptions(Local<Object>::Cast(info[1]));
  }

  std::string content = readFileContents(std::string(*filePath));
  if (content.empty()) {
    std::string path(*filePath);
    std::ifstream testFile(path.c_str());
    if (!testFile.is_open()) {
      Nan::ThrowError("Could not open file");
      return;
    }
    testFile.close();
  }

  std::vector<std::vector<std::string>> rows = parse(content, opts);
  info.GetReturnValue().Set(rowsToJsArray(rows));
}

NAN_METHOD(Stringify) {
  if (info.Length() < 1 || !info[0]->IsArray()) {
    Nan::ThrowTypeError("First argument must be an array");
    return;
  }

  Local<Array> arr = Local<Array>::Cast(info[0]);
  StringifyOptions opts;

  if (info.Length() >= 2 && info[1]->IsObject()) {
    opts = extractStringifyOptions(Local<Object>::Cast(info[1]));
  }

  std::vector<std::vector<std::string>> rows = jsArrayToRows(arr);
  std::string result = stringify(rows, opts);

  info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
}

NAN_METHOD(WriteFile) {
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsString()) {
    Nan::ThrowTypeError("Arguments must be (filePath, content)");
    return;
  }

  Nan::Utf8String filePath(info[0]);
  Nan::Utf8String content(info[1]);

  bool success = writeFile(std::string(*filePath), std::string(*content));
  info.GetReturnValue().Set(Nan::New(success));
}

void InitCsvParser(Local<Object> exports) {
  Nan::Set(exports, Nan::New("csvParse").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Parse)).ToLocalChecked());

  Nan::Set(exports, Nan::New("csvParseFile").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ParseFile)).ToLocalChecked());

  Nan::Set(exports, Nan::New("csvStringify").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Stringify)).ToLocalChecked());

  Nan::Set(exports, Nan::New("csvWriteFile").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(WriteFile)).ToLocalChecked());
}
