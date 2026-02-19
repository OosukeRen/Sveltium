#include "addon_api.h"
#include <fstream>
#include "rss_parser.h"

using namespace rssparser;

static ADDON_OBJECT_TYPE feedItemToJsObject(const FeedItem& item) {
  ADDON_OBJECT_TYPE obj = ADDON_OBJECT();

  ADDON_SET(obj, "title", ADDON_STRING(item.title));
  ADDON_SET(obj, "description", ADDON_STRING(item.description));
  ADDON_SET(obj, "link", ADDON_STRING(item.link));
  ADDON_SET(obj, "pubDate", ADDON_STRING(item.pubDate));
  ADDON_SET(obj, "author", ADDON_STRING(item.author));
  ADDON_SET(obj, "guid", ADDON_STRING(item.guid));

  // Convert categories vector to JS array
  ADDON_ARRAY_TYPE categories = ADDON_ARRAY(item.categories.size());
  for (size_t i = 0; i < item.categories.size(); i++) {
    ADDON_SET_INDEX(categories, i, ADDON_STRING(item.categories[i]));
  }
  ADDON_SET(obj, "categories", categories);

  return obj;
}

static ADDON_OBJECT_TYPE feedToJsObject(const Feed& feed) {
  ADDON_OBJECT_TYPE obj = ADDON_OBJECT();

  ADDON_SET(obj, "title", ADDON_STRING(feed.title));
  ADDON_SET(obj, "description", ADDON_STRING(feed.description));
  ADDON_SET(obj, "link", ADDON_STRING(feed.link));
  ADDON_SET(obj, "language", ADDON_STRING(feed.language));
  ADDON_SET(obj, "lastBuildDate", ADDON_STRING(feed.lastBuildDate));

  // Convert items vector to JS array
  ADDON_ARRAY_TYPE items = ADDON_ARRAY(feed.items.size());
  for (size_t i = 0; i < feed.items.size(); i++) {
    ADDON_SET_INDEX(items, i, feedItemToJsObject(feed.items[i]));
  }
  ADDON_SET(obj, "items", items);

  return obj;
}

ADDON_METHOD(RssParse) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be an XML string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(xml, ADDON_ARG(0));
  Feed feed = parse(std::string(ADDON_UTF8_VALUE(xml)));
  ADDON_RETURN(feedToJsObject(feed));
}

ADDON_METHOD(RssParseFile) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("First argument must be a file path string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(filePath, ADDON_ARG(0));
  std::string path(ADDON_UTF8_VALUE(filePath));

  // Check if file exists
  std::ifstream testFile(path.c_str());
  if (!testFile.is_open()) {
    ADDON_THROW_ERROR("Could not open file");
    ADDON_VOID_RETURN();
  }
  testFile.close();

  Feed feed = parseFile(path);
  ADDON_RETURN(feedToJsObject(feed));
}

void InitRssParser(ADDON_INIT_PARAMS) {
  ADDON_EXPORT_FUNCTION(exports, "rssParse", RssParse);
  ADDON_EXPORT_FUNCTION(exports, "rssParseFile", RssParseFile);
}
