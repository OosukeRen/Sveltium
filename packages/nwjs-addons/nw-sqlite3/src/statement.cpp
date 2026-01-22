#include "statement.h"
#include "database.h"
#include <stdexcept>
#include <cstring>

namespace nw_sqlite3 {

Statement::Statement(Database* db, const std::string& sql)
  : db_(db)
  , source_(sql)
  , stmt_(NULL)
  , isReader_(false)
  , hasRun_(false)
{
  if (!db || !db->isOpen()) {
    throw std::runtime_error("Database is closed");
  }

  int rc = sqlite3_prepare_v2(db->handle(), sql.c_str(), -1, &stmt_, NULL);

  if (rc != SQLITE_OK) {
    throw std::runtime_error(db->getError());
  }

  // Determine if this is a reader (SELECT, PRAGMA, etc.)
  // by checking if it returns columns
  isReader_ = (sqlite3_column_count(stmt_) > 0);
}

Statement::~Statement() {
  finalize();
}

Statement::Statement(const Statement& other) {
  db_ = other.db_;
  source_ = other.source_;
  stmt_ = NULL;
  isReader_ = other.isReader_;
  hasRun_ = false;
}

Statement& Statement::operator=(const Statement& other) {
  if (this != &other) {
    finalize();
    db_ = other.db_;
    source_ = other.source_;
    stmt_ = NULL;
    isReader_ = other.isReader_;
    hasRun_ = false;
  }
  return *this;
}

void Statement::checkValid() const {
  if (!stmt_) {
    throw std::runtime_error("Statement has been finalized");
  }
}

void Statement::bindInt(int index, int value) {
  checkValid();
  int rc = sqlite3_bind_int(stmt_, index, value);
  if (rc != SQLITE_OK) {
    throw std::runtime_error(db_->getError());
  }
}

void Statement::bindInt64(int index, int64_t value) {
  checkValid();
  int rc = sqlite3_bind_int64(stmt_, index, value);
  if (rc != SQLITE_OK) {
    throw std::runtime_error(db_->getError());
  }
}

void Statement::bindDouble(int index, double value) {
  checkValid();
  int rc = sqlite3_bind_double(stmt_, index, value);
  if (rc != SQLITE_OK) {
    throw std::runtime_error(db_->getError());
  }
}

void Statement::bindText(int index, const std::string& value) {
  checkValid();
  int rc = sqlite3_bind_text(stmt_, index, value.c_str(),
                             static_cast<int>(value.length()), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    throw std::runtime_error(db_->getError());
  }
}

void Statement::bindBlob(int index, const void* data, size_t size) {
  checkValid();
  int rc = sqlite3_bind_blob(stmt_, index, data,
                             static_cast<int>(size), SQLITE_TRANSIENT);
  if (rc != SQLITE_OK) {
    throw std::runtime_error(db_->getError());
  }
}

void Statement::bindNull(int index) {
  checkValid();
  int rc = sqlite3_bind_null(stmt_, index);
  if (rc != SQLITE_OK) {
    throw std::runtime_error(db_->getError());
  }
}

int Statement::getParameterIndex(const std::string& name) const {
  checkValid();
  return sqlite3_bind_parameter_index(stmt_, name.c_str());
}

bool Statement::step() {
  checkValid();
  hasRun_ = true;

  int rc = sqlite3_step(stmt_);

  if (rc == SQLITE_ROW) {
    return true;
  } else if (rc == SQLITE_DONE) {
    return false;
  } else {
    throw std::runtime_error(db_->getError());
  }
}

void Statement::reset() {
  checkValid();
  sqlite3_reset(stmt_);
  hasRun_ = false;
}

void Statement::clearBindings() {
  checkValid();
  sqlite3_clear_bindings(stmt_);
}

void Statement::finalize() {
  if (stmt_) {
    sqlite3_finalize(stmt_);
    stmt_ = NULL;
  }
}

int Statement::columnCount() const {
  checkValid();
  return sqlite3_column_count(stmt_);
}

std::string Statement::columnName(int index) const {
  checkValid();
  const char* name = sqlite3_column_name(stmt_, index);
  return name ? name : "";
}

int Statement::columnType(int index) const {
  checkValid();
  return sqlite3_column_type(stmt_, index);
}

int Statement::getInt(int index) const {
  checkValid();
  return sqlite3_column_int(stmt_, index);
}

int64_t Statement::getInt64(int index) const {
  checkValid();
  return sqlite3_column_int64(stmt_, index);
}

double Statement::getDouble(int index) const {
  checkValid();
  return sqlite3_column_double(stmt_, index);
}

std::string Statement::getText(int index) const {
  checkValid();
  const unsigned char* text = sqlite3_column_text(stmt_, index);
  int len = sqlite3_column_bytes(stmt_, index);
  if (text && len > 0) {
    return std::string(reinterpret_cast<const char*>(text), len);
  }
  return "";
}

const void* Statement::getBlob(int index, int* size) const {
  checkValid();
  *size = sqlite3_column_bytes(stmt_, index);
  return sqlite3_column_blob(stmt_, index);
}

bool Statement::isNull(int index) const {
  checkValid();
  return sqlite3_column_type(stmt_, index) == SQLITE_NULL;
}

int Statement::changes() const {
  if (!db_) return 0;
  return sqlite3_changes(db_->handle());
}

int64_t Statement::lastInsertRowid() const {
  if (!db_) return 0;
  return sqlite3_last_insert_rowid(db_->handle());
}

} // namespace nw_sqlite3
