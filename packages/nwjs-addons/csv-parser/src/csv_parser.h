#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>

namespace csvparser {

struct ParseOptions {
  char delimiter;
  char quote;
  char escape;
  bool skipEmptyLines;
  bool trim;

  ParseOptions() :
    delimiter(','),
    quote('"'),
    escape('"'),
    skipEmptyLines(true),
    trim(false) {}
};

struct StringifyOptions {
  char delimiter;
  char quote;
  bool quoteAll;
  std::string lineEnding;

  StringifyOptions() :
    delimiter(','),
    quote('"'),
    quoteAll(false),
    lineEnding("\r\n") {}
};

// Parse CSV string into rows of fields
std::vector<std::vector<std::string>> parse(
  const std::string& input,
  const ParseOptions& options
);

// Parse CSV file into rows of fields
std::vector<std::vector<std::string>> parseFile(
  const std::string& filePath,
  const ParseOptions& options
);

// Convert rows of fields to CSV string
std::string stringify(
  const std::vector<std::vector<std::string>>& data,
  const StringifyOptions& options
);

// Write rows to CSV file
bool writeFile(
  const std::string& filePath,
  const std::string& content
);

// Helper to read file contents
std::string readFileContents(const std::string& filePath);

} // namespace csvparser

#endif // CSV_PARSER_H
