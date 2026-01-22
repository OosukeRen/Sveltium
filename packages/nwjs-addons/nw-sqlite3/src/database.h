#ifndef NW_SQLITE3_DATABASE_H
#define NW_SQLITE3_DATABASE_H

#include <string>
#include "sqlite3.h"

namespace nw_sqlite3 {

/**
 * SQLite Database wrapper
 * Provides a clean C++ interface over SQLite3
 */
class Database {
public:
  /**
   * Open or create database
   * @param path Path to database file (":memory:" for in-memory)
   * @param readonly Open in read-only mode
   */
  Database(const std::string& path, bool readonly = false);
  ~Database();

  // Prevent copying
  Database(const Database&);
  Database& operator=(const Database&);

  /**
   * Check if database is open
   */
  bool isOpen() const { return db_ != NULL; }

  /**
   * Check if currently in a transaction
   */
  bool inTransaction() const;

  /**
   * Get database path
   */
  const std::string& path() const { return path_; }

  /**
   * Execute SQL that doesn't return results
   * @param sql SQL statement(s) to execute
   */
  void exec(const std::string& sql);

  /**
   * Close the database
   */
  void close();

  /**
   * Get SQLite handle (for Statement class)
   */
  sqlite3* handle() { return db_; }

  /**
   * Get last error message
   */
  std::string getError() const;

  /**
   * Get last insert rowid
   */
  int64_t lastInsertRowid() const;

  /**
   * Get number of changes from last statement
   */
  int changes() const;

  /**
   * Get total changes since open
   */
  int totalChanges() const;

private:
  std::string path_;
  sqlite3* db_;
};

} // namespace nw_sqlite3

#endif // NW_SQLITE3_DATABASE_H
