#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <string>
#include <vector>

namespace clipboard {

enum ClipboardType {
  CLIPBOARD_EMPTY,
  CLIPBOARD_TEXT,
  CLIPBOARD_FILES,
  CLIPBOARD_IMAGE,
  CLIPBOARD_UNKNOWN
};

// Type detection
ClipboardType getType();
bool hasText();
bool hasFiles();
bool hasImage();

// Text operations
std::string getText();
bool copyText(const std::string& text);

// File operations
std::vector<std::string> getFiles();
bool copyFiles(const std::vector<std::string>& paths);
bool cutFiles(const std::vector<std::string>& paths);
std::vector<std::string> pasteFiles(const std::string& destDir);
bool isCutOperation();

// Image operations
bool saveImageToFile(const std::string& filePath);
bool getImageSize(int& width, int& height);

// Clear clipboard
bool clear();

} // namespace clipboard

#endif // CLIPBOARD_H
