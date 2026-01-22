#include <nan.h>
#include "clipboard.h"

using namespace v8;
using namespace clipboard;

NAN_METHOD(GetType) {
  ClipboardType type = getType();

  const char* typeStr;
  switch (type) {
    case CLIPBOARD_EMPTY:   typeStr = "empty"; break;
    case CLIPBOARD_TEXT:    typeStr = "text"; break;
    case CLIPBOARD_FILES:   typeStr = "files"; break;
    case CLIPBOARD_IMAGE:   typeStr = "image"; break;
    default:                typeStr = "unknown"; break;
  }

  info.GetReturnValue().Set(Nan::New(typeStr).ToLocalChecked());
}

NAN_METHOD(HasText) {
  info.GetReturnValue().Set(Nan::New(hasText()));
}

NAN_METHOD(HasFiles) {
  info.GetReturnValue().Set(Nan::New(hasFiles()));
}

NAN_METHOD(HasImage) {
  info.GetReturnValue().Set(Nan::New(hasImage()));
}

NAN_METHOD(GetText) {
  std::string text = getText();
  if (text.empty()) {
    info.GetReturnValue().SetNull();
  } else {
    info.GetReturnValue().Set(Nan::New(text).ToLocalChecked());
  }
}

NAN_METHOD(CopyText) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Argument must be a string");
    return;
  }

  Nan::Utf8String text(info[0]);
  bool success = copyText(std::string(*text));
  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(GetFiles) {
  std::vector<std::string> files = getFiles();

  Local<Array> result = Nan::New<Array>(static_cast<uint32_t>(files.size()));

  for (size_t i = 0; i < files.size(); i++) {
    Nan::Set(result, static_cast<uint32_t>(i), Nan::New(files[i]).ToLocalChecked());
  }

  info.GetReturnValue().Set(result);
}

NAN_METHOD(CopyFiles) {
  if (info.Length() < 1 || !info[0]->IsArray()) {
    Nan::ThrowTypeError("Argument must be an array of strings");
    return;
  }

  Local<Array> arr = Local<Array>::Cast(info[0]);
  std::vector<std::string> paths;

  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> val = Nan::Get(arr, i).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      paths.push_back(std::string(*str));
    }
  }

  bool success = copyFiles(paths);
  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(CutFiles) {
  if (info.Length() < 1 || !info[0]->IsArray()) {
    Nan::ThrowTypeError("Argument must be an array of strings");
    return;
  }

  Local<Array> arr = Local<Array>::Cast(info[0]);
  std::vector<std::string> paths;

  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> val = Nan::Get(arr, i).ToLocalChecked();
    if (val->IsString()) {
      Nan::Utf8String str(val);
      paths.push_back(std::string(*str));
    }
  }

  bool success = cutFiles(paths);
  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(PasteFiles) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Argument must be a string (destination directory)");
    return;
  }

  Nan::Utf8String destDir(info[0]);
  std::vector<std::string> newPaths = pasteFiles(std::string(*destDir));

  Local<Array> result = Nan::New<Array>(static_cast<uint32_t>(newPaths.size()));

  for (size_t i = 0; i < newPaths.size(); i++) {
    Nan::Set(result, static_cast<uint32_t>(i), Nan::New(newPaths[i]).ToLocalChecked());
  }

  info.GetReturnValue().Set(result);
}

NAN_METHOD(IsCutOperation) {
  info.GetReturnValue().Set(Nan::New(isCutOperation()));
}

NAN_METHOD(GetImageSize) {
  int width = 0;
  int height = 0;

  if (getImageSize(width, height)) {
    Local<Object> result = Nan::New<Object>();
    Nan::Set(result, Nan::New("width").ToLocalChecked(), Nan::New(width));
    Nan::Set(result, Nan::New("height").ToLocalChecked(), Nan::New(height));
    info.GetReturnValue().Set(result);
  } else {
    info.GetReturnValue().SetNull();
  }
}

NAN_METHOD(SaveImageToFile) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Argument must be a string (file path)");
    return;
  }

  Nan::Utf8String filePath(info[0]);
  bool success = saveImageToFile(std::string(*filePath));
  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(Clear) {
  bool success = clear();
  info.GetReturnValue().Set(Nan::New(success));
}

void InitClipboard(Local<Object> exports) {
  Nan::Set(exports, Nan::New("clipboardGetType").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetType)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardHasText").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(HasText)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardHasFiles").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(HasFiles)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardHasImage").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(HasImage)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardGetText").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetText)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardCopyText").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(CopyText)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardGetFiles").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetFiles)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardCopyFiles").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(CopyFiles)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardCutFiles").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(CutFiles)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardPasteFiles").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(PasteFiles)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardIsCutOperation").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(IsCutOperation)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardGetImageSize").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetImageSize)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardSaveImageToFile").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(SaveImageToFile)).ToLocalChecked());

  Nan::Set(exports, Nan::New("clipboardClear").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Clear)).ToLocalChecked());
}
