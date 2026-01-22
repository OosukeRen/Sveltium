#ifndef NW_SQLITE3_STATEMENT_H
#define NW_SQLITE3_STATEMENT_H

#include <string>
#include <vector>
#include "sqlite3.h"

namespace nw_sqlite3 {

class Database;

/**
 * Column information
 */
struct ColumnInfo {
  std::string name;
  int type;
};

/**
 * SQLite Statement wrapper
 * Represents a prepared SQL statement
 */
class Statement {
public:
  /**
   * Prepare a statement
   * @param db Database to prepare statement for
   * @param sql SQL statement
   */
  Statement(Database* db, const std::string& sql);
  ~Statement();

  // Prevent copying
  Statement(const Statement&);
  Statement& operator=(const Statement&);

  /**
   * Check if statement is a reader (SELECT, etc.)
   */
  bool isReader() const { return isReader_; }

  /**
   * Get SQL source
   */
  const std::string& source() const { return source_; }

  /**
   * Bind parameters by index (1-based)
   */
  void bindInt(int index, int value);
  void bindInt64(int index, int64_t value);
  void bindDouble(int index, double value);
  void bindText(int index, const std::string& value);
  void bindBlob(int index, const void* data, size_t size);
  void bindNull(int index);

  /**
   * Bind by parameter name (@name, :name, $name)
   */
  int getParameterIndex(const std::string& name) const;

  /**
   * Step to next row
   * @returns true if has row, false if done
   */
  bool step();

  /**
   * Reset statement for re-execution
   */
  void reset();

  /**
   * Clear bindings
   */
  void clearBindings();

  /**
   * Finalize statement (release resources)
   */
  void finalize();

  /**
   * Get column count
   */
  int columnCount() const;

  /**
   * Get column name
   */
  std::string columnName(int index) const;

  /**
   * Get column type (SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, SQLITE_NULL)
   */
  int columnType(int index) const;

  /**
   * Get column values (0-based index)
   */
  int getInt(int index) const;
  int64_t getInt64(int index) const;
  double getDouble(int index) const;
  std::string getText(int index) const;
  const void* getBlob(int index, int* size) const;
  bool isNull(int index) const;

  /**
   * Get changes count from last execution
   */
  int changes() const;

  /**
   * Get last insert rowid
   */
  int64_t lastInsertRowid() const;

  /**
   * Check if statement is valid
   */
  bool isValid() const { return stmt_ != NULL; }

private:
  Database* db_;
  std::string source_;
  sqlite3_stmt* stmt_;
  bool isReader_;
  bool hasRun_;

  void checkValid() const;
};

} // namespace nw_sqlite3

#endif // NW_SQLITE3_STATEMENT_H
