#include "addon_api.h"
#include "database.h"
#include "statement.h"

using namespace nw_sqlite3;

// Forward declarations
class StatementWrap;

// Database wrapper
class DatabaseWrap : public ADDON_OBJECT_WRAP {
public:
  static void Init(ADDON_INIT_PARAMS);
  static ADDON_METHOD(New);
  static ADDON_METHOD(Exec);
  static ADDON_METHOD(Prepare);
  static ADDON_METHOD(Close);
  static ADDON_GETTER(GetOpen);
  static ADDON_GETTER(GetPath);
  static ADDON_GETTER(GetInTransaction);

  static ADDON_PERSISTENT_FUNCTION constructor;

  Database* db_;

private:
  DatabaseWrap() : db_(NULL) {}
  ~DatabaseWrap() {
    if (db_) {
      delete db_;
      db_ = NULL;
    }
  }
};

ADDON_PERSISTENT_FUNCTION DatabaseWrap::constructor;

// Statement wrapper
class StatementWrap : public ADDON_OBJECT_WRAP {
public:
  static void Init(ADDON_INIT_PARAMS);
  static ADDON_OBJECT_TYPE Create(Statement* stmt);
  static ADDON_METHOD(Run);
  static ADDON_METHOD(Get);
  static ADDON_METHOD(All);
  static ADDON_METHOD(Reset);
  static ADDON_METHOD(Finalize);
  static ADDON_GETTER(GetSource);
  static ADDON_GETTER(GetReader);

  static ADDON_PERSISTENT_TEMPLATE constructorTemplate;
  static ADDON_PERSISTENT_FUNCTION constructor;

  Statement* stmt_;

private:
  StatementWrap() : stmt_(NULL) {}
  ~StatementWrap() {
    if (stmt_) {
      delete stmt_;
      stmt_ = NULL;
    }
  }

  // Bind JS value to statement parameter
  static void bindValue(Statement* stmt, int index, ADDON_VALUE val);

  // Convert row to JS object
  static ADDON_OBJECT_TYPE rowToObject(Statement* stmt);
};

ADDON_PERSISTENT_TEMPLATE StatementWrap::constructorTemplate;
ADDON_PERSISTENT_FUNCTION StatementWrap::constructor;

// ============================================
// Database Implementation
// ============================================

void DatabaseWrap::Init(ADDON_INIT_PARAMS) {
  ADDON_HANDLE_SCOPE();

  auto tpl = ADDON_NEW_CTOR_TEMPLATE_WITH(New);
  ADDON_SET_CLASS_NAME(tpl, "Database");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  // Methods
  ADDON_SET_PROTOTYPE_METHOD(tpl, "exec", Exec);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "prepare", Prepare);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "close", Close);

  // Accessors
  ADDON_SET_ACCESSOR(tpl, "open", GetOpen);
  ADDON_SET_ACCESSOR(tpl, "path", GetPath);
  ADDON_SET_ACCESSOR(tpl, "inTransaction", GetInTransaction);

  ADDON_PERSISTENT_RESET(constructor, ADDON_GET_CTOR_FUNCTION(tpl));
  ADDON_SET(exports, "Database", ADDON_GET_CTOR_FUNCTION(tpl));
}

ADDON_METHOD(DatabaseWrap::New) {
  ADDON_ENV;
  if (!ADDON_IS_CONSTRUCT_CALL()) {
    ADDON_THROW_ERROR("Use 'new' to create Database");
    ADDON_VOID_RETURN();
  }

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Path must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(path, ADDON_ARG(0));
  bool readonly = false;

  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_OBJECT(ADDON_ARG(1))) {
    ADDON_OBJECT_TYPE opts = ADDON_AS_OBJECT(ADDON_ARG(1));
    ADDON_VALUE roVal = ADDON_GET(opts, "readonly");
    if (ADDON_IS_BOOLEAN(roVal)) {
      readonly = ADDON_BOOL_VALUE(roVal);
    }
  }

  try {
    DatabaseWrap* wrap = new DatabaseWrap();
    wrap->db_ = new Database(ADDON_UTF8_VALUE(path), readonly);
    wrap->Wrap(ADDON_THIS());
    ADDON_RETURN(ADDON_THIS());
  } catch (const std::exception& e) {
    ADDON_THROW_ERROR(e.what());
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(DatabaseWrap::Exec) {
  ADDON_ENV;
  DatabaseWrap* wrap = ADDON_UNWRAP(DatabaseWrap, ADDON_HOLDER());

  if (!wrap->db_ || !wrap->db_->isOpen()) {
    ADDON_THROW_ERROR("Database is closed");
    ADDON_VOID_RETURN();
  }

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("SQL must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(sql, ADDON_ARG(0));

  try {
    wrap->db_->exec(ADDON_UTF8_VALUE(sql));
    ADDON_RETURN(ADDON_HOLDER());
  } catch (const std::exception& e) {
    ADDON_THROW_ERROR(e.what());
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(DatabaseWrap::Prepare) {
  ADDON_ENV;
  DatabaseWrap* wrap = ADDON_UNWRAP(DatabaseWrap, ADDON_HOLDER());

  if (!wrap->db_ || !wrap->db_->isOpen()) {
    ADDON_THROW_ERROR("Database is closed");
    ADDON_VOID_RETURN();
  }

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("SQL must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(sql, ADDON_ARG(0));

  try {
    Statement* stmt = new Statement(wrap->db_, ADDON_UTF8_VALUE(sql));
    ADDON_OBJECT_TYPE stmtObj = StatementWrap::Create(stmt);
    ADDON_RETURN(stmtObj);
  } catch (const std::exception& e) {
    ADDON_THROW_ERROR(e.what());
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(DatabaseWrap::Close) {
  ADDON_ENV;
  DatabaseWrap* wrap = ADDON_UNWRAP(DatabaseWrap, ADDON_HOLDER());

  if (wrap->db_) {
    wrap->db_->close();
  }
  ADDON_VOID_RETURN();
}

ADDON_GETTER(DatabaseWrap::GetOpen) {
  ADDON_ENV;
  DatabaseWrap* wrap = ADDON_UNWRAP(DatabaseWrap, ADDON_HOLDER());
  bool isOpen = wrap->db_ && wrap->db_->isOpen();
  ADDON_RETURN(ADDON_BOOLEAN(isOpen));
}

ADDON_GETTER(DatabaseWrap::GetPath) {
  ADDON_ENV;
  DatabaseWrap* wrap = ADDON_UNWRAP(DatabaseWrap, ADDON_HOLDER());
  if (wrap->db_) {
    ADDON_RETURN(ADDON_STRING(wrap->db_->path().c_str()));
  }
  ADDON_RETURN_NULL();
}

ADDON_GETTER(DatabaseWrap::GetInTransaction) {
  ADDON_ENV;
  DatabaseWrap* wrap = ADDON_UNWRAP(DatabaseWrap, ADDON_HOLDER());
  bool inTx = wrap->db_ && wrap->db_->inTransaction();
  ADDON_RETURN(ADDON_BOOLEAN(inTx));
}

// ============================================
// Statement Implementation
// ============================================

void StatementWrap::Init(ADDON_INIT_PARAMS) {
  ADDON_HANDLE_SCOPE();

  auto tpl = ADDON_NEW_CTOR_TEMPLATE();
  ADDON_SET_CLASS_NAME(tpl, "Statement");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  ADDON_SET_PROTOTYPE_METHOD(tpl, "run", Run);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "get", Get);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "all", All);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "reset", Reset);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "finalize", Finalize);

  ADDON_SET_ACCESSOR(tpl, "source", GetSource);
  ADDON_SET_ACCESSOR(tpl, "reader", GetReader);

  ADDON_PERSISTENT_RESET(constructorTemplate, tpl);
  ADDON_PERSISTENT_RESET(constructor, ADDON_GET_CTOR_FUNCTION(tpl));
}

ADDON_OBJECT_TYPE StatementWrap::Create(Statement* stmt) {
  ADDON_ESCAPABLE_SCOPE();

  auto cons = ADDON_PERSISTENT_GET(constructor);
  ADDON_OBJECT_TYPE instance = ADDON_NEW_INSTANCE(cons);

  StatementWrap* wrap = new StatementWrap();
  wrap->stmt_ = stmt;
  wrap->Wrap(instance);

  return ADDON_ESCAPE(instance);
}

void StatementWrap::bindValue(Statement* stmt, int index, ADDON_VALUE val) {
  if (ADDON_IS_NULL(val) || ADDON_IS_UNDEFINED(val)) {
    stmt->bindNull(index);
  }
  else if (ADDON_IS_BOOLEAN(val)) {
    stmt->bindInt(index, ADDON_BOOL_VALUE(val) ? 1 : 0);
  }
  else if (ADDON_IS_INT32(val)) {
    stmt->bindInt(index, ADDON_TO_INT32(val));
  }
  else if (ADDON_IS_NUMBER(val)) {
    double num = ADDON_TO_DOUBLE(val);
    // Check if it's an integer that fits in int64
    if (num == static_cast<double>(static_cast<int64_t>(num))) {
      stmt->bindInt64(index, static_cast<int64_t>(num));
    } else {
      stmt->bindDouble(index, num);
    }
  }
  else if (ADDON_IS_STRING(val)) {
    ADDON_UTF8(str, val);
    stmt->bindText(index, ADDON_UTF8_VALUE(str));
  }
  else if (ADDON_BUFFER_IS(val)) {
    ADDON_OBJECT_TYPE buf = ADDON_AS_OBJECT(val);
    stmt->bindBlob(index, ADDON_BUFFER_DATA(buf), ADDON_BUFFER_LENGTH(buf));
  }
  else {
    stmt->bindNull(index);
  }
}

ADDON_OBJECT_TYPE StatementWrap::rowToObject(Statement* stmt) {
  ADDON_ESCAPABLE_SCOPE();

  ADDON_OBJECT_TYPE row = ADDON_OBJECT();
  int colCount = stmt->columnCount();

  for (int i = 0; i < colCount; i++) {
    std::string name = stmt->columnName(i);
    int type = stmt->columnType(i);
    ADDON_VALUE value;

    switch (type) {
      case SQLITE_INTEGER:
        value = ADDON_NUMBER(stmt->getInt64(i));
        break;

      case SQLITE_FLOAT:
        value = ADDON_NUMBER(stmt->getDouble(i));
        break;

      case SQLITE_TEXT:
        value = ADDON_STRING(stmt->getText(i).c_str());
        break;

      case SQLITE_BLOB: {
        int size;
        const void* data = stmt->getBlob(i, &size);
        value = ADDON_COPY_BUFFER(static_cast<const char*>(data), size);
        break;
      }

      case SQLITE_NULL:
      default:
        value = ADDON_NULL();
        break;
    }

    ADDON_SET(row, name.c_str(), value);
  }

  return ADDON_ESCAPE(row);
}

ADDON_METHOD(StatementWrap::Run) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  if (!wrap->stmt_ || !wrap->stmt_->isValid()) {
    ADDON_THROW_ERROR("Statement has been finalized");
    ADDON_VOID_RETURN();
  }

  try {
    wrap->stmt_->reset();
    wrap->stmt_->clearBindings();

    // Bind parameters
    for (int i = 0; i < ADDON_ARG_COUNT(); i++) {
      bindValue(wrap->stmt_, i + 1, ADDON_ARG(i));
    }

    // Execute
    wrap->stmt_->step();

    // Return result object
    ADDON_OBJECT_TYPE result = ADDON_OBJECT();
    ADDON_SET(result, "changes", ADDON_INTEGER(wrap->stmt_->changes()));
    ADDON_SET(result, "lastInsertRowid",
              ADDON_NUMBER(wrap->stmt_->lastInsertRowid()));

    ADDON_RETURN(result);
  } catch (const std::exception& e) {
    ADDON_THROW_ERROR(e.what());
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(StatementWrap::Get) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  if (!wrap->stmt_ || !wrap->stmt_->isValid()) {
    ADDON_THROW_ERROR("Statement has been finalized");
    ADDON_VOID_RETURN();
  }

  try {
    wrap->stmt_->reset();
    wrap->stmt_->clearBindings();

    // Bind parameters
    for (int i = 0; i < ADDON_ARG_COUNT(); i++) {
      bindValue(wrap->stmt_, i + 1, ADDON_ARG(i));
    }

    // Get first row
    if (wrap->stmt_->step()) {
      ADDON_RETURN(rowToObject(wrap->stmt_));
    }
    ADDON_RETURN_UNDEFINED();
  } catch (const std::exception& e) {
    ADDON_THROW_ERROR(e.what());
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(StatementWrap::All) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  if (!wrap->stmt_ || !wrap->stmt_->isValid()) {
    ADDON_THROW_ERROR("Statement has been finalized");
    ADDON_VOID_RETURN();
  }

  try {
    wrap->stmt_->reset();
    wrap->stmt_->clearBindings();

    // Bind parameters
    for (int i = 0; i < ADDON_ARG_COUNT(); i++) {
      bindValue(wrap->stmt_, i + 1, ADDON_ARG(i));
    }

    // Get all rows
    ADDON_ARRAY_TYPE rows = ADDON_ARRAY_EMPTY();
    uint32_t index = 0;

    while (wrap->stmt_->step()) {
      ADDON_SET_INDEX(rows, index++, rowToObject(wrap->stmt_));
    }

    ADDON_RETURN(rows);
  } catch (const std::exception& e) {
    ADDON_THROW_ERROR(e.what());
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(StatementWrap::Reset) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  if (wrap->stmt_ && wrap->stmt_->isValid()) {
    wrap->stmt_->reset();
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(StatementWrap::Finalize) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  if (wrap->stmt_) {
    wrap->stmt_->finalize();
  }
  ADDON_VOID_RETURN();
}

ADDON_GETTER(StatementWrap::GetSource) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  if (wrap->stmt_) {
    ADDON_RETURN(ADDON_STRING(wrap->stmt_->source().c_str()));
  }
  ADDON_RETURN_NULL();
}

ADDON_GETTER(StatementWrap::GetReader) {
  ADDON_ENV;
  StatementWrap* wrap = ADDON_UNWRAP(StatementWrap, ADDON_HOLDER());

  bool isReader = wrap->stmt_ && wrap->stmt_->isReader();
  ADDON_RETURN(ADDON_BOOLEAN(isReader));
}

// ============================================
// Module Initialization
// ============================================

void InitSQLite3(ADDON_INIT_PARAMS) {
  ADDON_OBJECT_TYPE sqlite3 = ADDON_OBJECT();

  DatabaseWrap::Init(sqlite3);
  StatementWrap::Init(sqlite3);

  ADDON_SET(exports, "sqlite3", sqlite3);
}
