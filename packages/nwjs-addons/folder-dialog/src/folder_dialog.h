#ifndef FOLDER_DIALOG_H
#define FOLDER_DIALOG_H

#include <string>
#include <vector>

namespace folder_dialog {

struct Options {
  std::string title;
  std::string initialPath;
};

struct FileOptions {
  std::string title;
  std::string initialPath;
  std::string defaultName;          // Default filename for save dialog
  std::vector<std::string> filters; // Filter strings like "Text Files|*.txt|All Files|*.*"
  bool multiSelect;                 // Allow multiple file selection (open only)
};

// Opens folder browser dialog, returns selected path or empty string if cancelled
std::string open(const Options& options);

// Opens file open dialog, returns selected path(s) or empty if cancelled
std::vector<std::string> openFile(const FileOptions& options);

// Opens file save dialog, returns selected path or empty if cancelled
std::string saveFile(const FileOptions& options);

} // namespace folder_dialog

#endif // FOLDER_DIALOG_H
