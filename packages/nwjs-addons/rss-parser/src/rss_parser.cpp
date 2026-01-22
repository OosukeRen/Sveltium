#include "rss_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace rssparser {

// Helper: trim whitespace from string
static std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) {
    return "";
  }
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

// Helper: unescape XML entities
static std::string unescapeXml(const std::string& str) {
  std::string result = str;

  // Replace common XML entities
  size_t pos = 0;
  while ((pos = result.find("&lt;", pos)) != std::string::npos) {
    result.replace(pos, 4, "<");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&gt;", pos)) != std::string::npos) {
    result.replace(pos, 4, ">");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&amp;", pos)) != std::string::npos) {
    result.replace(pos, 5, "&");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&quot;", pos)) != std::string::npos) {
    result.replace(pos, 6, "\"");
    pos += 1;
  }
  pos = 0;
  while ((pos = result.find("&apos;", pos)) != std::string::npos) {
    result.replace(pos, 6, "'");
    pos += 1;
  }

  return result;
}

// Helper: extract content between tags, handling CDATA
static std::string extractTagContent(const std::string& xml, const std::string& tagName, size_t startPos = 0) {
  std::string openTag = "<" + tagName;
  std::string closeTag = "</" + tagName + ">";

  size_t tagStart = xml.find(openTag, startPos);
  if (tagStart == std::string::npos) {
    return "";
  }

  // Find the end of opening tag (handle attributes)
  size_t contentStart = xml.find(">", tagStart);
  if (contentStart == std::string::npos) {
    return "";
  }

  // Check for self-closing tag
  if (xml[contentStart - 1] == '/') {
    return "";
  }

  contentStart++;

  size_t contentEnd = xml.find(closeTag, contentStart);
  if (contentEnd == std::string::npos) {
    return "";
  }

  std::string content = xml.substr(contentStart, contentEnd - contentStart);

  // Handle CDATA sections
  size_t cdataStart = content.find("<![CDATA[");
  if (cdataStart != std::string::npos) {
    size_t cdataEnd = content.find("]]>", cdataStart);
    if (cdataEnd != std::string::npos) {
      content = content.substr(cdataStart + 9, cdataEnd - cdataStart - 9);
      return trim(content);
    }
  }

  return trim(unescapeXml(content));
}

// Helper: extract attribute value from a tag
static std::string extractAttribute(const std::string& xml, const std::string& tagName, const std::string& attrName, size_t startPos = 0) {
  std::string openTag = "<" + tagName;

  size_t tagStart = xml.find(openTag, startPos);
  if (tagStart == std::string::npos) {
    return "";
  }

  size_t tagEnd = xml.find(">", tagStart);
  if (tagEnd == std::string::npos) {
    return "";
  }

  std::string tagContent = xml.substr(tagStart, tagEnd - tagStart + 1);
  std::string attrSearch = attrName + "=\"";

  size_t attrStart = tagContent.find(attrSearch);
  if (attrStart == std::string::npos) {
    // Try single quotes
    attrSearch = attrName + "='";
    attrStart = tagContent.find(attrSearch);
  }

  if (attrStart == std::string::npos) {
    return "";
  }

  attrStart += attrSearch.length();
  char quote = tagContent[attrStart - 1];
  size_t attrEnd = tagContent.find(quote, attrStart);

  if (attrEnd == std::string::npos) {
    return "";
  }

  return unescapeXml(tagContent.substr(attrStart, attrEnd - attrStart));
}

// Helper: find all occurrences of a tag and extract content
static std::vector<std::string> findAllTags(const std::string& xml, const std::string& tagName) {
  std::vector<std::string> results;
  std::string openTag = "<" + tagName;
  std::string closeTag = "</" + tagName + ">";

  size_t pos = 0;
  while (true) {
    size_t tagStart = xml.find(openTag, pos);
    if (tagStart == std::string::npos) {
      break;
    }

    size_t contentStart = xml.find(">", tagStart);
    if (contentStart == std::string::npos) {
      break;
    }

    // Check for self-closing tag
    if (xml[contentStart - 1] == '/') {
      pos = contentStart + 1;
      continue;
    }

    contentStart++;

    size_t contentEnd = xml.find(closeTag, contentStart);
    if (contentEnd == std::string::npos) {
      break;
    }

    std::string content = xml.substr(contentStart, contentEnd - contentStart);
    results.push_back(content);

    pos = contentEnd + closeTag.length();
  }

  return results;
}

// Helper: check if XML is Atom format
static bool isAtomFeed(const std::string& xml) {
  size_t feedPos = xml.find("<feed");
  size_t rssPos = xml.find("<rss");

  if (feedPos != std::string::npos && (rssPos == std::string::npos || feedPos < rssPos)) {
    return true;
  }
  return false;
}

// Parse RSS 2.0 item
static FeedItem parseRssItem(const std::string& itemXml) {
  FeedItem item;

  item.title = extractTagContent(itemXml, "title");
  item.description = extractTagContent(itemXml, "description");
  item.link = extractTagContent(itemXml, "link");
  item.pubDate = extractTagContent(itemXml, "pubDate");
  item.author = extractTagContent(itemXml, "author");

  // Try dc:creator if author is empty
  if (item.author.empty()) {
    item.author = extractTagContent(itemXml, "dc:creator");
  }

  item.guid = extractTagContent(itemXml, "guid");

  // Extract categories
  std::vector<std::string> categories = findAllTags(itemXml, "category");
  for (size_t i = 0; i < categories.size(); i++) {
    std::string cat = trim(unescapeXml(categories[i]));
    if (!cat.empty()) {
      item.categories.push_back(cat);
    }
  }

  return item;
}

// Parse Atom entry
static FeedItem parseAtomEntry(const std::string& entryXml) {
  FeedItem item;

  item.title = extractTagContent(entryXml, "title");

  // Atom uses content or summary for description
  item.description = extractTagContent(entryXml, "content");
  if (item.description.empty()) {
    item.description = extractTagContent(entryXml, "summary");
  }

  // Atom link is in href attribute
  item.link = extractAttribute(entryXml, "link", "href");

  // Atom uses updated or published for date
  item.pubDate = extractTagContent(entryXml, "updated");
  if (item.pubDate.empty()) {
    item.pubDate = extractTagContent(entryXml, "published");
  }

  // Atom author is nested
  std::string authorXml = "";
  std::vector<std::string> authors = findAllTags(entryXml, "author");
  if (!authors.empty()) {
    item.author = extractTagContent(authors[0], "name");
  }

  item.guid = extractTagContent(entryXml, "id");

  // Extract categories from category tags
  std::string openCat = "<category";
  size_t pos = 0;
  while (true) {
    size_t catPos = entryXml.find(openCat, pos);
    if (catPos == std::string::npos) {
      break;
    }

    std::string term = extractAttribute(entryXml, "category", "term", pos);
    if (!term.empty()) {
      item.categories.push_back(term);
    }

    pos = catPos + 1;
  }

  return item;
}

// Parse RSS 2.0 feed
static Feed parseRss(const std::string& xml) {
  Feed feed;

  // Extract channel info
  std::string channel = "";
  std::vector<std::string> channels = findAllTags(xml, "channel");
  if (!channels.empty()) {
    channel = channels[0];
  }

  if (!channel.empty()) {
    feed.title = extractTagContent(channel, "title");
    feed.description = extractTagContent(channel, "description");
    feed.link = extractTagContent(channel, "link");
    feed.language = extractTagContent(channel, "language");
    feed.lastBuildDate = extractTagContent(channel, "lastBuildDate");

    // Parse items
    std::vector<std::string> items = findAllTags(channel, "item");
    for (size_t i = 0; i < items.size(); i++) {
      feed.items.push_back(parseRssItem(items[i]));
    }
  }

  return feed;
}

// Parse Atom feed
static Feed parseAtom(const std::string& xml) {
  Feed feed;

  feed.title = extractTagContent(xml, "title");
  feed.description = extractTagContent(xml, "subtitle");
  feed.link = extractAttribute(xml, "link", "href");
  feed.lastBuildDate = extractTagContent(xml, "updated");

  // Parse entries
  std::vector<std::string> entries = findAllTags(xml, "entry");
  for (size_t i = 0; i < entries.size(); i++) {
    feed.items.push_back(parseAtomEntry(entries[i]));
  }

  return feed;
}

std::string readFileContents(const std::string& filePath) {
  std::ifstream file(filePath.c_str(), std::ios::binary);
  if (!file.is_open()) {
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();

  // Skip UTF-8 BOM if present
  if (content.length() >= 3 &&
      (unsigned char)content[0] == 0xEF &&
      (unsigned char)content[1] == 0xBB &&
      (unsigned char)content[2] == 0xBF) {
    content = content.substr(3);
  }

  return content;
}

Feed parse(const std::string& xml) {
  if (isAtomFeed(xml)) {
    return parseAtom(xml);
  }
  return parseRss(xml);
}

Feed parseFile(const std::string& filePath) {
  std::string content = readFileContents(filePath);
  return parse(content);
}

} // namespace rssparser
