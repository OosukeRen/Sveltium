#include "folder_dialog.h"
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

namespace folder_dialog {

// Callback for setting initial path in legacy dialog
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
  if (uMsg == BFFM_INITIALIZED && lpData != 0) {
    SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, lpData);
  }
  return 0;
}

// Convert UTF-8 to wide string
static std::wstring utf8ToWide(const std::string& utf8) {
  if (utf8.empty()) {
    return std::wstring();
  }
  int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
  std::wstring wide;
  wide.resize(wideLen);
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], wideLen);
  return wide;
}

// Convert wide string to UTF-8
static std::string wideToUtf8(const std::wstring& wide) {
  if (wide.empty()) {
    return std::string();
  }
  int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
  std::string utf8;
  utf8.resize(utf8Len - 1);
  WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], utf8Len, NULL, NULL);
  return utf8;
}

// Try modern IFileDialog (Vista+)
// Returns: result path, sets apiAvailable to true if the API exists (even if user cancelled)
static std::string openModern(const Options& options, bool& apiAvailable) {
  std::string result;
  apiAvailable = false;

  IFileDialog* pfd = NULL;
  HRESULT hr = CoCreateInstance(
    CLSID_FileOpenDialog,
    NULL,
    CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&pfd)
  );

  if (FAILED(hr)) {
    // Modern API not available (likely XP)
    return result;
  }

  // Modern API is available
  apiAvailable = true;

  // Set options for folder picker
  DWORD dwOptions = 0;
  pfd->GetOptions(&dwOptions);
  pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

  // Set title if provided
  if (!options.title.empty()) {
    std::wstring wTitle = utf8ToWide(options.title);
    pfd->SetTitle(wTitle.c_str());
  }

  // Set initial folder if provided
  if (!options.initialPath.empty()) {
    std::wstring wPath = utf8ToWide(options.initialPath);
    IShellItem* psi = NULL;
    hr = SHCreateItemFromParsingName(wPath.c_str(), NULL, IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr)) {
      pfd->SetFolder(psi);
      psi->Release();
    }
  }

  // Show dialog
  hr = pfd->Show(NULL);

  if (SUCCEEDED(hr)) {
    IShellItem* psi = NULL;
    hr = pfd->GetResult(&psi);
    if (SUCCEEDED(hr)) {
      PWSTR pszPath = NULL;
      hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
      if (SUCCEEDED(hr)) {
        std::wstring widePath(pszPath);
        result = wideToUtf8(widePath);
        CoTaskMemFree(pszPath);
      }
      psi->Release();
    }
  }

  pfd->Release();
  return result;
}

// Legacy SHBrowseForFolder (XP compatible)
static std::string openLegacy(const Options& options) {
  std::string result;

  std::wstring wTitle = options.title.empty()
    ? L"Select Folder"
    : utf8ToWide(options.title);

  std::wstring wInitialPath = utf8ToWide(options.initialPath);

  BROWSEINFOW bi = { 0 };
  bi.lpszTitle = wTitle.c_str();
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
  bi.lpfn = BrowseCallbackProc;
  bi.lParam = wInitialPath.empty()
    ? 0
    : reinterpret_cast<LPARAM>(wInitialPath.c_str());

  LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

  if (pidl != NULL) {
    WCHAR path[MAX_PATH];
    if (SHGetPathFromIDListW(pidl, path)) {
      result = wideToUtf8(path);
    }
    CoTaskMemFree(pidl);
  }

  return result;
}

std::string open(const Options& options) {
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

  std::string result;
  bool modernApiAvailable = false;

  // Try modern dialog first (Vista+)
  result = openModern(options, modernApiAvailable);

  // Only fall back to legacy if modern API is not available (XP)
  // Don't show legacy dialog if user cancelled the modern one
  if (!modernApiAvailable) {
    result = openLegacy(options);
  }

  CoUninitialize();
  return result;
}

// Build filter string for file dialogs
// Input: ["Text Files", "*.txt", "All Files", "*.*"]
// Output: "Text Files\0*.txt\0All Files\0*.*\0\0"
static std::wstring buildFilterString(const std::vector<std::string>& filters) {
  std::wstring result;
  for (size_t i = 0; i < filters.size(); i++) {
    result += utf8ToWide(filters[i]);
    result += L'\0';
  }
  result += L'\0';
  return result;
}

std::vector<std::string> openFile(const FileOptions& options) {
  std::vector<std::string> results;

  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

  // Try modern IFileOpenDialog (Vista+)
  IFileOpenDialog* pfd = NULL;
  HRESULT hr = CoCreateInstance(
    CLSID_FileOpenDialog,
    NULL,
    CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&pfd)
  );

  if (SUCCEEDED(hr)) {
    // Modern dialog available
    DWORD dwOptions = 0;
    pfd->GetOptions(&dwOptions);

    DWORD newOptions = dwOptions | FOS_FORCEFILESYSTEM;
    if (options.multiSelect) {
      newOptions |= FOS_ALLOWMULTISELECT;
    }
    pfd->SetOptions(newOptions);

    // Set title
    if (!options.title.empty()) {
      std::wstring wTitle = utf8ToWide(options.title);
      pfd->SetTitle(wTitle.c_str());
    }

    // Set initial folder
    if (!options.initialPath.empty()) {
      std::wstring wPath = utf8ToWide(options.initialPath);
      IShellItem* psi = NULL;
      hr = SHCreateItemFromParsingName(wPath.c_str(), NULL, IID_PPV_ARGS(&psi));
      if (SUCCEEDED(hr)) {
        pfd->SetFolder(psi);
        psi->Release();
      }
    }

    // Set file type filters
    if (!options.filters.empty() && options.filters.size() >= 2) {
      size_t filterCount = options.filters.size() / 2;
      COMDLG_FILTERSPEC* fileTypes = new COMDLG_FILTERSPEC[filterCount];
      std::vector<std::wstring> wideStrings;

      for (size_t i = 0; i < filterCount; i++) {
        wideStrings.push_back(utf8ToWide(options.filters[i * 2]));
        wideStrings.push_back(utf8ToWide(options.filters[i * 2 + 1]));
        fileTypes[i].pszName = wideStrings[i * 2].c_str();
        fileTypes[i].pszSpec = wideStrings[i * 2 + 1].c_str();
      }

      pfd->SetFileTypes(static_cast<UINT>(filterCount), fileTypes);
      delete[] fileTypes;
    }

    hr = pfd->Show(NULL);

    if (SUCCEEDED(hr)) {
      if (options.multiSelect) {
        IShellItemArray* pItems = NULL;
        hr = pfd->GetResults(&pItems);
        if (SUCCEEDED(hr)) {
          DWORD count = 0;
          pItems->GetCount(&count);
          for (DWORD i = 0; i < count; i++) {
            IShellItem* psi = NULL;
            if (SUCCEEDED(pItems->GetItemAt(i, &psi))) {
              PWSTR pszPath = NULL;
              if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                results.push_back(wideToUtf8(pszPath));
                CoTaskMemFree(pszPath);
              }
              psi->Release();
            }
          }
          pItems->Release();
        }
      } else {
        IShellItem* psi = NULL;
        hr = pfd->GetResult(&psi);
        if (SUCCEEDED(hr)) {
          PWSTR pszPath = NULL;
          if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
            results.push_back(wideToUtf8(pszPath));
            CoTaskMemFree(pszPath);
          }
          psi->Release();
        }
      }
    }

    pfd->Release();
  } else {
    // Fall back to legacy GetOpenFileName (XP)
    WCHAR szFile[MAX_PATH * 100] = { 0 };

    std::wstring wTitle = options.title.empty()
      ? L"Open File"
      : utf8ToWide(options.title);

    std::wstring wFilter = options.filters.empty()
      ? L"All Files\0*.*\0\0"
      : buildFilterString(options.filters);

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = wFilter.c_str();
    ofn.lpstrTitle = wTitle.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (options.multiSelect) {
      ofn.Flags |= OFN_ALLOWMULTISELECT;
    }

    if (!options.initialPath.empty()) {
      std::wstring wInitial = utf8ToWide(options.initialPath);
      ofn.lpstrInitialDir = wInitial.c_str();
    }

    if (GetOpenFileNameW(&ofn)) {
      if (options.multiSelect) {
        // Multiple files: directory + null + file1 + null + file2 + null + null
        WCHAR* p = szFile;
        std::wstring dir = p;
        p += wcslen(p) + 1;

        if (*p == L'\0') {
          // Only one file selected
          results.push_back(wideToUtf8(dir));
        } else {
          while (*p != L'\0') {
            std::wstring fullPath = dir + L"\\" + p;
            results.push_back(wideToUtf8(fullPath));
            p += wcslen(p) + 1;
          }
        }
      } else {
        results.push_back(wideToUtf8(szFile));
      }
    }
  }

  CoUninitialize();
  return results;
}

std::string saveFile(const FileOptions& options) {
  std::string result;

  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

  // Try modern IFileSaveDialog (Vista+)
  IFileSaveDialog* pfd = NULL;
  HRESULT hr = CoCreateInstance(
    CLSID_FileSaveDialog,
    NULL,
    CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&pfd)
  );

  if (SUCCEEDED(hr)) {
    // Modern dialog available
    DWORD dwOptions = 0;
    pfd->GetOptions(&dwOptions);
    pfd->SetOptions(dwOptions | FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT);

    // Set title
    if (!options.title.empty()) {
      std::wstring wTitle = utf8ToWide(options.title);
      pfd->SetTitle(wTitle.c_str());
    }

    // Set default filename
    if (!options.defaultName.empty()) {
      std::wstring wName = utf8ToWide(options.defaultName);
      pfd->SetFileName(wName.c_str());
    }

    // Set initial folder
    if (!options.initialPath.empty()) {
      std::wstring wPath = utf8ToWide(options.initialPath);
      IShellItem* psi = NULL;
      hr = SHCreateItemFromParsingName(wPath.c_str(), NULL, IID_PPV_ARGS(&psi));
      if (SUCCEEDED(hr)) {
        pfd->SetFolder(psi);
        psi->Release();
      }
    }

    // Set file type filters
    if (!options.filters.empty() && options.filters.size() >= 2) {
      size_t filterCount = options.filters.size() / 2;
      COMDLG_FILTERSPEC* fileTypes = new COMDLG_FILTERSPEC[filterCount];
      std::vector<std::wstring> wideStrings;

      for (size_t i = 0; i < filterCount; i++) {
        wideStrings.push_back(utf8ToWide(options.filters[i * 2]));
        wideStrings.push_back(utf8ToWide(options.filters[i * 2 + 1]));
        fileTypes[i].pszName = wideStrings[i * 2].c_str();
        fileTypes[i].pszSpec = wideStrings[i * 2 + 1].c_str();
      }

      pfd->SetFileTypes(static_cast<UINT>(filterCount), fileTypes);
      delete[] fileTypes;
    }

    hr = pfd->Show(NULL);

    if (SUCCEEDED(hr)) {
      IShellItem* psi = NULL;
      hr = pfd->GetResult(&psi);
      if (SUCCEEDED(hr)) {
        PWSTR pszPath = NULL;
        if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
          result = wideToUtf8(pszPath);
          CoTaskMemFree(pszPath);
        }
        psi->Release();
      }
    }

    pfd->Release();
  } else {
    // Fall back to legacy GetSaveFileName (XP)
    WCHAR szFile[MAX_PATH] = { 0 };

    if (!options.defaultName.empty()) {
      std::wstring wName = utf8ToWide(options.defaultName);
      wcscpy_s(szFile, MAX_PATH, wName.c_str());
    }

    std::wstring wTitle = options.title.empty()
      ? L"Save File"
      : utf8ToWide(options.title);

    std::wstring wFilter = options.filters.empty()
      ? L"All Files\0*.*\0\0"
      : buildFilterString(options.filters);

    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = wFilter.c_str();
    ofn.lpstrTitle = wTitle.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

    if (!options.initialPath.empty()) {
      std::wstring wInitial = utf8ToWide(options.initialPath);
      ofn.lpstrInitialDir = wInitial.c_str();
    }

    if (GetSaveFileNameW(&ofn)) {
      result = wideToUtf8(szFile);
    }
  }

  CoUninitialize();
  return result;
}

} // namespace folder_dialog
