#include <nan.h>
#include <fstream>
#include "rss_parser.h"

using namespace v8;
using namespace rssparser;

static Local<Object> feedItemToJsObject(const FeedItem& item) {
  Local<Object> obj = Nan::New<Object>();

  Nan::Set(obj, Nan::New("title").ToLocalChecked(),
    Nan::New(item.title).ToLocalChecked());

  Nan::Set(obj, Nan::New("description").ToLocalChecked(),
    Nan::New(item.description).ToLocalChecked());

  Nan::Set(obj, Nan::New("link").ToLocalChecked(),
    Nan::New(item.link).ToLocalChecked());

  Nan::Set(obj, Nan::New("pubDate").ToLocalChecked(),
    Nan::New(item.pubDate).ToLocalChecked());

  Nan::Set(obj, Nan::New("author").ToLocalChecked(),
    Nan::New(item.author).ToLocalChecked());

  Nan::Set(obj, Nan::New("guid").ToLocalChecked(),
    Nan::New(item.guid).ToLocalChecked());

  // Convert categories vector to JS array
  Local<Array> categories = Nan::New<Array>(static_cast<uint32_t>(item.categories.size()));
  for (size_t i = 0; i < item.categories.size(); i++) {
    Nan::Set(categories, static_cast<uint32_t>(i),
      Nan::New(item.categories[i]).ToLocalChecked());
  }
  Nan::Set(obj, Nan::New("categories").ToLocalChecked(), categories);

  return obj;
}

static Local<Object> feedToJsObject(const Feed& feed) {
  Local<Object> obj = Nan::New<Object>();

  Nan::Set(obj, Nan::New("title").ToLocalChecked(),
    Nan::New(feed.title).ToLocalChecked());

  Nan::Set(obj, Nan::New("description").ToLocalChecked(),
    Nan::New(feed.description).ToLocalChecked());

  Nan::Set(obj, Nan::New("link").ToLocalChecked(),
    Nan::New(feed.link).ToLocalChecked());

  Nan::Set(obj, Nan::New("language").ToLocalChecked(),
    Nan::New(feed.language).ToLocalChecked());

  Nan::Set(obj, Nan::New("lastBuildDate").ToLocalChecked(),
    Nan::New(feed.lastBuildDate).ToLocalChecked());

  // Convert items vector to JS array
  Local<Array> items = Nan::New<Array>(static_cast<uint32_t>(feed.items.size()));
  for (size_t i = 0; i < feed.items.size(); i++) {
    Nan::Set(items, static_cast<uint32_t>(i), feedItemToJsObject(feed.items[i]));
  }
  Nan::Set(obj, Nan::New("items").ToLocalChecked(), items);

  return obj;
}

NAN_METHOD(RssParse) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be an XML string");
    return;
  }

  Nan::Utf8String xml(info[0]);
  Feed feed = parse(std::string(*xml));
  info.GetReturnValue().Set(feedToJsObject(feed));
}

NAN_METHOD(RssParseFile) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("First argument must be a file path string");
    return;
  }

  Nan::Utf8String filePath(info[0]);
  std::string path(*filePath);

  // Check if file exists
  std::ifstream testFile(path.c_str());
  if (!testFile.is_open()) {
    Nan::ThrowError("Could not open file");
    return;
  }
  testFile.close();

  Feed feed = parseFile(path);
  info.GetReturnValue().Set(feedToJsObject(feed));
}

void InitRssParser(Local<Object> exports) {
  Nan::Set(exports, Nan::New("rssParse").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(RssParse)).ToLocalChecked());

  Nan::Set(exports, Nan::New("rssParseFile").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(RssParseFile)).ToLocalChecked());
}
