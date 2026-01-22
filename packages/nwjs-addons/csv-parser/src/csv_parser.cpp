#include "csv_parser.h"
#include <fstream>
#include <sstream>

namespace csvparser {

enum ParserState {
  STATE_FIELD_START,
  STATE_UNQUOTED_FIELD,
  STATE_QUOTED_FIELD,
  STATE_QUOTE_IN_QUOTED
};

static std::string trimString(const std::string& str) {
  size_t start = 0;
  size_t end = str.length();

  while (start < end && (str[start] == ' ' || str[start] == '\t')) {
    start++;
  }
  while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t')) {
    end--;
  }

  return str.substr(start, end - start);
}

static bool isRowEmpty(const std::vector<std::string>& row) {
  if (row.empty()) {
    return true;
  }

  for (size_t i = 0; i < row.size(); i++) {
    if (!row[i].empty()) {
      return false;
    }
  }

  return true;
}

std::vector<std::vector<std::string>> parse(
  const std::string& input,
  const ParseOptions& options
) {
  std::vector<std::vector<std::string>> result;
  std::vector<std::string> currentRow;
  std::string currentField;
  ParserState state = STATE_FIELD_START;

  size_t i = 0;
  size_t len = input.length();

  // Skip UTF-8 BOM if present
  if (len >= 3 &&
      (unsigned char)input[0] == 0xEF &&
      (unsigned char)input[1] == 0xBB &&
      (unsigned char)input[2] == 0xBF) {
    i = 3;
  }

  while (i < len) {
    char c = input[i];
    char nextChar = (i + 1 < len) ? input[i + 1] : '\0';

    switch (state) {
      case STATE_FIELD_START:
        if (c == options.quote) {
          state = STATE_QUOTED_FIELD;
          i++;
        } else if (c == options.delimiter) {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          currentField.clear();
          i++;
        } else if (c == '\r') {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
          if (!skipRow) {
            result.push_back(currentRow);
          }
          currentRow.clear();
          currentField.clear();
          i++;
          if (nextChar == '\n') {
            i++;
          }
        } else if (c == '\n') {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
          if (!skipRow) {
            result.push_back(currentRow);
          }
          currentRow.clear();
          currentField.clear();
          i++;
        } else {
          currentField += c;
          state = STATE_UNQUOTED_FIELD;
          i++;
        }
        break;

      case STATE_UNQUOTED_FIELD:
        if (c == options.delimiter) {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          currentField.clear();
          state = STATE_FIELD_START;
          i++;
        } else if (c == '\r') {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
          if (!skipRow) {
            result.push_back(currentRow);
          }
          currentRow.clear();
          currentField.clear();
          state = STATE_FIELD_START;
          i++;
          if (nextChar == '\n') {
            i++;
          }
        } else if (c == '\n') {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
          if (!skipRow) {
            result.push_back(currentRow);
          }
          currentRow.clear();
          currentField.clear();
          state = STATE_FIELD_START;
          i++;
        } else {
          currentField += c;
          i++;
        }
        break;

      case STATE_QUOTED_FIELD:
        if (c == options.escape && nextChar == options.quote) {
          currentField += options.quote;
          i += 2;
        } else if (c == options.quote) {
          state = STATE_QUOTE_IN_QUOTED;
          i++;
        } else {
          currentField += c;
          i++;
        }
        break;

      case STATE_QUOTE_IN_QUOTED:
        if (c == options.quote) {
          currentField += options.quote;
          state = STATE_QUOTED_FIELD;
          i++;
        } else if (c == options.delimiter) {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          currentField.clear();
          state = STATE_FIELD_START;
          i++;
        } else if (c == '\r') {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
          if (!skipRow) {
            result.push_back(currentRow);
          }
          currentRow.clear();
          currentField.clear();
          state = STATE_FIELD_START;
          i++;
          if (nextChar == '\n') {
            i++;
          }
        } else if (c == '\n') {
          std::string field = options.trim ? trimString(currentField) : currentField;
          currentRow.push_back(field);
          bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
          if (!skipRow) {
            result.push_back(currentRow);
          }
          currentRow.clear();
          currentField.clear();
          state = STATE_FIELD_START;
          i++;
        } else {
          currentField += c;
          state = STATE_UNQUOTED_FIELD;
          i++;
        }
        break;
    }
  }

  // Handle final field/row
  bool hasContent = !currentField.empty() || !currentRow.empty();
  if (hasContent) {
    std::string field = options.trim ? trimString(currentField) : currentField;
    currentRow.push_back(field);
    bool skipRow = options.skipEmptyLines && isRowEmpty(currentRow);
    if (!skipRow) {
      result.push_back(currentRow);
    }
  }

  return result;
}

std::string readFileContents(const std::string& filePath) {
  std::ifstream file(filePath.c_str(), std::ios::binary);
  if (!file.is_open()) {
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::vector<std::vector<std::string>> parseFile(
  const std::string& filePath,
  const ParseOptions& options
) {
  std::string content = readFileContents(filePath);
  return parse(content, options);
}

static bool needsQuoting(const std::string& field, const StringifyOptions& options) {
  if (options.quoteAll) {
    return true;
  }

  for (size_t i = 0; i < field.length(); i++) {
    char c = field[i];
    if (c == options.delimiter || c == options.quote || c == '\r' || c == '\n') {
      return true;
    }
  }

  return false;
}

static std::string escapeField(const std::string& field, const StringifyOptions& options) {
  std::string result;
  result.reserve(field.length() * 2);

  for (size_t i = 0; i < field.length(); i++) {
    char c = field[i];
    if (c == options.quote) {
      result += options.quote;
      result += options.quote;
    } else {
      result += c;
    }
  }

  return result;
}

std::string stringify(
  const std::vector<std::vector<std::string>>& data,
  const StringifyOptions& options
) {
  std::string result;

  for (size_t rowIndex = 0; rowIndex < data.size(); rowIndex++) {
    const std::vector<std::string>& row = data[rowIndex];

    for (size_t colIndex = 0; colIndex < row.size(); colIndex++) {
      if (colIndex > 0) {
        result += options.delimiter;
      }

      const std::string& field = row[colIndex];
      bool shouldQuote = needsQuoting(field, options);

      if (shouldQuote) {
        result += options.quote;
        result += escapeField(field, options);
        result += options.quote;
      } else {
        result += field;
      }
    }

    result += options.lineEnding;
  }

  return result;
}

bool writeFile(
  const std::string& filePath,
  const std::string& content
) {
  std::ofstream file(filePath.c_str(), std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  file << content;
  return file.good();
}

} // namespace csvparser
