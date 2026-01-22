#ifndef RSS_PARSER_H
#define RSS_PARSER_H

#include <string>
#include <vector>

namespace rssparser {

struct FeedItem {
  std::string title;
  std::string description;
  std::string link;
  std::string pubDate;
  std::string author;
  std::string guid;
  std::vector<std::string> categories;

  FeedItem() : title(""), description(""), link(""), pubDate(""), author(""), guid("") {}
};

struct Feed {
  std::string title;
  std::string description;
  std::string link;
  std::string language;
  std::string lastBuildDate;
  std::vector<FeedItem> items;

  Feed() : title(""), description(""), link(""), language(""), lastBuildDate("") {}
};

// Parse RSS/Atom XML string into Feed structure
Feed parse(const std::string& xml);

// Parse RSS/Atom file into Feed structure
Feed parseFile(const std::string& filePath);

// Read file contents with UTF-8 BOM handling
std::string readFileContents(const std::string& filePath);

} // namespace rssparser

#endif
