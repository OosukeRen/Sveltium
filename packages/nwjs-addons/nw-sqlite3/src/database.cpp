#include "database.h"
#include <stdexcept>

namespace nw_sqlite3 {

Database::Database(const std::string& path, bool readonly)
  : path_(path)
  , db_(NULL)
{
  int flags = SQLITE_OPEN_CREATE;

  if (readonly) {
    flags |= SQLITE_OPEN_READONLY;
  } else {
    flags |= SQLITE_OPEN_READWRITE;
  }

  int rc = sqlite3_open_v2(path.c_str(), &db_, flags, NULL);

  if (rc != SQLITE_OK) {
    std::string error = db_ ? sqlite3_errmsg(db_) : "Unknown error";
    if (db_) {
      sqlite3_close(db_);
      db_ = NULL;
    }
    throw std::runtime_error("Failed to open database: " + error);
  }

  // Enable extended result codes
  sqlite3_extended_result_codes(db_, 1);

  // Set busy timeout (5 seconds)
  sqlite3_busy_timeout(db_, 5000);
}

Database::~Database() {
  close();
}

Database::Database(const Database& other) {
  // Copy not allowed
  path_ = other.path_;
  db_ = NULL;
}

Database& Database::operator=(const Database& other) {
  if (this != &other) {
    close();
    path_ = other.path_;
    db_ = NULL;
  }
  return *this;
}

bool Database::inTransaction() const {
  if (!db_) return false;
  return sqlite3_get_autocommit(db_) == 0;
}

void Database::exec(const std::string& sql) {
  if (!db_) {
    throw std::runtime_error("Database is closed");
  }

  char* errMsg = NULL;
  int rc = sqlite3_exec(db_, sql.c_str(), NULL, NULL, &errMsg);

  if (rc != SQLITE_OK) {
    std::string error = errMsg ? errMsg : "Unknown error";
    if (errMsg) {
      sqlite3_free(errMsg);
    }
    throw std::runtime_error(error);
  }
}

void Database::close() {
  if (db_) {
    sqlite3_close_v2(db_);
    db_ = NULL;
  }
}

std::string Database::getError() const {
  if (!db_) return "Database is closed";
  return sqlite3_errmsg(db_);
}

int64_t Database::lastInsertRowid() const {
  if (!db_) return 0;
  return sqlite3_last_insert_rowid(db_);
}

int Database::changes() const {
  if (!db_) return 0;
  return sqlite3_changes(db_);
}

int Database::totalChanges() const {
  if (!db_) return 0;
  return sqlite3_total_changes(db_);
}

} // namespace nw_sqlite3
