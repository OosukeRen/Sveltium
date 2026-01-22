#include <nan.h>
#include "folder_dialog.h"

using namespace v8;
using namespace folder_dialog;

// Helper to parse FileOptions from JS object
static FileOptions parseFileOptions(Local<Object> optObj) {
  FileOptions opts;
  opts.multiSelect = false;

  Local<String> titleKey = Nan::New("title").ToLocalChecked();
  Local<String> initialPathKey = Nan::New("initialPath").ToLocalChecked();
  Local<String> defaultNameKey = Nan::New("defaultName").ToLocalChecked();
  Local<String> filtersKey = Nan::New("filters").ToLocalChecked();
  Local<String> multiSelectKey = Nan::New("multiSelect").ToLocalChecked();

  if (Nan::Has(optObj, titleKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, titleKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      opts.title = std::string(*str);
    }
  }

  if (Nan::Has(optObj, initialPathKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, initialPathKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      opts.initialPath = std::string(*str);
    }
  }

  if (Nan::Has(optObj, defaultNameKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, defaultNameKey).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      opts.defaultName = std::string(*str);
    }
  }

  if (Nan::Has(optObj, filtersKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, filtersKey).ToLocalChecked();
    if (val->IsArray()) {
      Local<Array> arr = Local<Array>::Cast(val);
      for (uint32_t i = 0; i < arr->Length(); i++) {
        Local<Value> item = Nan::Get(arr, i).ToLocalChecked();
        if (item->IsString()) {
          Nan::Utf8String str(item);
          opts.filters.push_back(std::string(*str));
        }
      }
    }
  }

  if (Nan::Has(optObj, multiSelectKey).FromJust()) {
    Local<Value> val = Nan::Get(optObj, multiSelectKey).ToLocalChecked();
    if (val->IsBoolean()) {
      opts.multiSelect = Nan::To<bool>(val).FromJust();
    }
  }

  return opts;
}

NAN_METHOD(Open) {
  Options opts;

  // Parse options object if provided
  if (info.Length() > 0 && info[0]->IsObject()) {
    Local<Object> optObj = Nan::To<Object>(info[0]).ToLocalChecked();

    Local<String> titleKey = Nan::New("title").ToLocalChecked();
    Local<String> initialPathKey = Nan::New("initialPath").ToLocalChecked();

    if (Nan::Has(optObj, titleKey).FromJust()) {
      Local<Value> titleVal = Nan::Get(optObj, titleKey).ToLocalChecked();
      if (titleVal->IsString()) {
        Nan::Utf8String title(titleVal);
        opts.title = std::string(*title);
      }
    }

    if (Nan::Has(optObj, initialPathKey).FromJust()) {
      Local<Value> pathVal = Nan::Get(optObj, initialPathKey).ToLocalChecked();
      if (pathVal->IsString()) {
        Nan::Utf8String path(pathVal);
        opts.initialPath = std::string(*path);
      }
    }
  }

  std::string result = open(opts);

  if (result.empty()) {
    info.GetReturnValue().SetNull();
  } else {
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  }
}

NAN_METHOD(OpenFile) {
  FileOptions opts;

  if (info.Length() > 0 && info[0]->IsObject()) {
    Local<Object> optObj = Nan::To<Object>(info[0]).ToLocalChecked();
    opts = parseFileOptions(optObj);
  }

  std::vector<std::string> results = openFile(opts);

  if (results.empty()) {
    info.GetReturnValue().SetNull();
  } else if (results.size() == 1 && !opts.multiSelect) {
    // Return single string for single file selection
    info.GetReturnValue().Set(Nan::New(results[0]).ToLocalChecked());
  } else {
    // Return array for multiselect
    Local<Array> arr = Nan::New<Array>(static_cast<uint32_t>(results.size()));
    for (size_t i = 0; i < results.size(); i++) {
      Nan::Set(arr, static_cast<uint32_t>(i), Nan::New(results[i]).ToLocalChecked());
    }
    info.GetReturnValue().Set(arr);
  }
}

NAN_METHOD(SaveFile) {
  FileOptions opts;

  if (info.Length() > 0 && info[0]->IsObject()) {
    Local<Object> optObj = Nan::To<Object>(info[0]).ToLocalChecked();
    opts = parseFileOptions(optObj);
  }

  std::string result = saveFile(opts);

  if (result.empty()) {
    info.GetReturnValue().SetNull();
  } else {
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  }
}

void InitFolderDialog(Local<Object> exports) {
  Nan::Set(exports, Nan::New("folderDialogOpen").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Open)).ToLocalChecked());

  Nan::Set(exports, Nan::New("fileDialogOpen").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(OpenFile)).ToLocalChecked());

  Nan::Set(exports, Nan::New("fileDialogSave").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(SaveFile)).ToLocalChecked());
}
