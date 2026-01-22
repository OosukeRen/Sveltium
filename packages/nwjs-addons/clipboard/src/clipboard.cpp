#include "clipboard.h"
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace clipboard {

// Helper to get GDI+ encoder CLSID
static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
  UINT num = 0;
  UINT size = 0;

  Gdiplus::GetImageEncodersSize(&num, &size);
  if (size == 0) {
    return -1;
  }

  Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL) {
    return -1;
  }

  Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

  int result = -1;
  for (UINT i = 0; i < num; ++i) {
    if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
      *pClsid = pImageCodecInfo[i].Clsid;
      result = i;
      break;
    }
  }

  free(pImageCodecInfo);
  return result;
}

ClipboardType getType() {
  if (!OpenClipboard(NULL)) {
    return CLIPBOARD_UNKNOWN;
  }

  ClipboardType result = CLIPBOARD_EMPTY;

  // Check in priority order (most common first)
  if (IsClipboardFormatAvailable(CF_UNICODETEXT) || IsClipboardFormatAvailable(CF_TEXT)) {
    result = CLIPBOARD_TEXT;
  }
  else if (IsClipboardFormatAvailable(CF_HDROP)) {
    result = CLIPBOARD_FILES;
  }
  else if (IsClipboardFormatAvailable(CF_BITMAP) || IsClipboardFormatAvailable(CF_DIB)) {
    result = CLIPBOARD_IMAGE;
  }

  CloseClipboard();
  return result;
}

bool hasText() {
  return getType() == CLIPBOARD_TEXT;
}

bool hasFiles() {
  return getType() == CLIPBOARD_FILES;
}

bool hasImage() {
  return getType() == CLIPBOARD_IMAGE;
}

std::string getText() {
  std::string result;

  if (!OpenClipboard(NULL)) {
    return result;
  }

  // Try Unicode first
  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (hData != NULL) {
    WCHAR* pszText = static_cast<WCHAR*>(GlobalLock(hData));
    if (pszText != NULL) {
      // Convert wide string to UTF-8
      int size = WideCharToMultiByte(CP_UTF8, 0, pszText, -1, NULL, 0, NULL, NULL);
      if (size > 0) {
        result.resize(size - 1);
        WideCharToMultiByte(CP_UTF8, 0, pszText, -1, &result[0], size, NULL, NULL);
      }
      GlobalUnlock(hData);
    }
  }
  else {
    // Fall back to ANSI
    hData = GetClipboardData(CF_TEXT);
    if (hData != NULL) {
      char* pszText = static_cast<char*>(GlobalLock(hData));
      if (pszText != NULL) {
        result = pszText;
        GlobalUnlock(hData);
      }
    }
  }

  CloseClipboard();
  return result;
}

bool copyText(const std::string& text) {
  if (!OpenClipboard(NULL)) {
    return false;
  }

  EmptyClipboard();

  // Convert UTF-8 to wide string
  int wideLen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
  if (wideLen == 0) {
    CloseClipboard();
    return false;
  }

  // Allocate global memory for wide text
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, wideLen * sizeof(WCHAR));
  if (hMem == NULL) {
    CloseClipboard();
    return false;
  }

  // Copy text to global memory
  WCHAR* pMem = static_cast<WCHAR*>(GlobalLock(hMem));
  MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, pMem, wideLen);
  GlobalUnlock(hMem);

  // Set clipboard data
  bool success = (SetClipboardData(CF_UNICODETEXT, hMem) != NULL);
  CloseClipboard();

  return success;
}

std::vector<std::string> getFiles() {
  std::vector<std::string> files;

  if (!OpenClipboard(NULL)) {
    return files;
  }

  HANDLE hData = GetClipboardData(CF_HDROP);
  if (hData != NULL) {
    HDROP hDrop = static_cast<HDROP>(hData);
    UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);

    for (UINT i = 0; i < count; i++) {
      WCHAR path[MAX_PATH];
      if (DragQueryFileW(hDrop, i, path, MAX_PATH) > 0) {
        // Convert wide string to UTF-8
        int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
        if (size > 0) {
          std::string utf8Path;
          utf8Path.resize(size - 1);
          WideCharToMultiByte(CP_UTF8, 0, path, -1, &utf8Path[0], size, NULL, NULL);
          files.push_back(utf8Path);
        }
      }
    }
  }

  CloseClipboard();
  return files;
}

// Helper to set files on clipboard with optional cut flag
static bool setFilesOnClipboard(const std::vector<std::string>& paths, bool cut) {
  if (paths.empty()) {
    return false;
  }

  if (!OpenClipboard(NULL)) {
    return false;
  }

  EmptyClipboard();

  // Calculate total size needed for DROPFILES structure
  size_t totalSize = sizeof(DROPFILES);
  std::vector<std::wstring> widePaths;

  for (size_t i = 0; i < paths.size(); i++) {
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, paths[i].c_str(), -1, NULL, 0);
    std::wstring widePath;
    widePath.resize(wideLen);
    MultiByteToWideChar(CP_UTF8, 0, paths[i].c_str(), -1, &widePath[0], wideLen);
    widePaths.push_back(widePath);
    totalSize += wideLen * sizeof(WCHAR);
  }
  totalSize += sizeof(WCHAR); // Double null terminator

  // Allocate global memory
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, totalSize);
  if (hMem == NULL) {
    CloseClipboard();
    return false;
  }

  // Fill DROPFILES structure
  DROPFILES* pDropFiles = static_cast<DROPFILES*>(GlobalLock(hMem));
  pDropFiles->pFiles = sizeof(DROPFILES);
  pDropFiles->fWide = TRUE;

  WCHAR* pPath = reinterpret_cast<WCHAR*>(reinterpret_cast<BYTE*>(pDropFiles) + sizeof(DROPFILES));
  for (size_t i = 0; i < widePaths.size(); i++) {
    wcscpy(pPath, widePaths[i].c_str());
    pPath += widePaths[i].length();
  }

  GlobalUnlock(hMem);

  // Set CF_HDROP
  bool success = (SetClipboardData(CF_HDROP, hMem) != NULL);

  // Set preferred drop effect (copy or cut)
  if (success) {
    UINT dropEffectFormat = RegisterClipboardFormatA("Preferred DropEffect");
    HGLOBAL hDropEffect = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
    if (hDropEffect != NULL) {
      DWORD* pDropEffect = static_cast<DWORD*>(GlobalLock(hDropEffect));
      *pDropEffect = cut ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
      GlobalUnlock(hDropEffect);
      SetClipboardData(dropEffectFormat, hDropEffect);
    }
  }

  CloseClipboard();
  return success;
}

bool copyFiles(const std::vector<std::string>& paths) {
  return setFilesOnClipboard(paths, false);
}

bool cutFiles(const std::vector<std::string>& paths) {
  return setFilesOnClipboard(paths, true);
}

bool isCutOperation() {
  bool isCut = false;

  if (!OpenClipboard(NULL)) {
    return false;
  }

  UINT dropEffectFormat = RegisterClipboardFormatA("Preferred DropEffect");
  HANDLE hData = GetClipboardData(dropEffectFormat);

  if (hData != NULL) {
    DWORD* pDropEffect = static_cast<DWORD*>(GlobalLock(hData));
    if (pDropEffect != NULL) {
      isCut = (*pDropEffect & DROPEFFECT_MOVE) != 0;
      GlobalUnlock(hData);
    }
  }

  CloseClipboard();
  return isCut;
}

std::vector<std::string> pasteFiles(const std::string& destDir) {
  std::vector<std::string> newPaths;
  std::vector<std::string> srcFiles = getFiles();

  if (srcFiles.empty()) {
    return newPaths;
  }

  bool isCut = isCutOperation();

  // Convert destination to wide string
  int destWideLen = MultiByteToWideChar(CP_UTF8, 0, destDir.c_str(), -1, NULL, 0);
  std::wstring wideDestDir;
  wideDestDir.resize(destWideLen);
  MultiByteToWideChar(CP_UTF8, 0, destDir.c_str(), -1, &wideDestDir[0], destWideLen);

  for (size_t i = 0; i < srcFiles.size(); i++) {
    // Convert source to wide string
    int srcWideLen = MultiByteToWideChar(CP_UTF8, 0, srcFiles[i].c_str(), -1, NULL, 0);
    std::wstring wideSrcPath;
    wideSrcPath.resize(srcWideLen);
    MultiByteToWideChar(CP_UTF8, 0, srcFiles[i].c_str(), -1, &wideSrcPath[0], srcWideLen);

    // Get filename
    const WCHAR* filename = wcsrchr(wideSrcPath.c_str(), L'\\');
    if (filename == NULL) {
      filename = wideSrcPath.c_str();
    } else {
      filename++; // Skip backslash
    }

    // Build destination path
    std::wstring wideNewPath = wideDestDir;
    if (wideNewPath.back() != L'\\') {
      wideNewPath += L'\\';
    }
    wideNewPath += filename;

    // Prepare double-null terminated strings for SHFileOperation
    std::wstring srcDouble = wideSrcPath;
    srcDouble.push_back(L'\0');
    std::wstring dstDouble = wideNewPath;
    dstDouble.push_back(L'\0');

    SHFILEOPSTRUCTW op = { 0 };
    op.wFunc = isCut ? FO_MOVE : FO_COPY;
    op.pFrom = srcDouble.c_str();
    op.pTo = dstDouble.c_str();
    op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

    if (SHFileOperationW(&op) == 0 && !op.fAnyOperationsAborted) {
      // Convert new path to UTF-8
      int size = WideCharToMultiByte(CP_UTF8, 0, wideNewPath.c_str(), -1, NULL, 0, NULL, NULL);
      if (size > 0) {
        std::string utf8NewPath;
        utf8NewPath.resize(size - 1);
        WideCharToMultiByte(CP_UTF8, 0, wideNewPath.c_str(), -1, &utf8NewPath[0], size, NULL, NULL);
        newPaths.push_back(utf8NewPath);
      }
    }
  }

  return newPaths;
}

bool getImageSize(int& width, int& height) {
  width = 0;
  height = 0;

  if (!OpenClipboard(NULL)) {
    return false;
  }

  bool success = false;
  HANDLE hData = GetClipboardData(CF_DIB);

  if (hData != NULL) {
    BITMAPINFOHEADER* pBmi = static_cast<BITMAPINFOHEADER*>(GlobalLock(hData));
    if (pBmi != NULL) {
      width = pBmi->biWidth;
      height = abs(pBmi->biHeight);
      success = true;
      GlobalUnlock(hData);
    }
  }

  CloseClipboard();
  return success;
}

bool saveImageToFile(const std::string& filePath) {
  if (!OpenClipboard(NULL)) {
    return false;
  }

  bool success = false;

  // Get bitmap from clipboard
  HBITMAP hBitmap = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));

  if (hBitmap != NULL) {
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Create Bitmap from HBITMAP
    Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);

    if (pBitmap != NULL) {
      // Determine format from extension
      const WCHAR* mimeType = L"image/png";
      std::string ext = filePath.substr(filePath.find_last_of('.') + 1);
      if (ext == "jpg" || ext == "jpeg") {
        mimeType = L"image/jpeg";
      } else if (ext == "bmp") {
        mimeType = L"image/bmp";
      } else if (ext == "gif") {
        mimeType = L"image/gif";
      }

      // Get encoder CLSID
      CLSID clsid;
      if (GetEncoderClsid(mimeType, &clsid) >= 0) {
        // Convert file path to wide string
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
        std::wstring widePath;
        widePath.resize(wideLen);
        MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &widePath[0], wideLen);

        // Save image
        success = (pBitmap->Save(widePath.c_str(), &clsid, NULL) == Gdiplus::Ok);
      }

      delete pBitmap;
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
  }

  CloseClipboard();
  return success;
}

bool clear() {
  if (!OpenClipboard(NULL)) {
    return false;
  }

  BOOL success = EmptyClipboard();
  CloseClipboard();
  return success != 0;
}

} // namespace clipboard
