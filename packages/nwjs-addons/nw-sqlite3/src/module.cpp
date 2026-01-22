#include <nan.h>
#include "database.h"
#include "statement.h"

using namespace v8;
using namespace nw_sqlite3;

// Forward declarations
class StatementWrap;

// Database wrapper
class DatabaseWrap : public Nan::ObjectWrap {
public:
  static void Init(Local<Object> exports);
  static NAN_METHOD(New);
  static NAN_METHOD(Exec);
  static NAN_METHOD(Prepare);
  static NAN_METHOD(Close);
  static NAN_GETTER(GetOpen);
  static NAN_GETTER(GetPath);
  static NAN_GETTER(GetInTransaction);

  static Nan::Persistent<Function> constructor;

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

Nan::Persistent<Function> DatabaseWrap::constructor;

// Statement wrapper
class StatementWrap : public Nan::ObjectWrap {
public:
  static void Init(Local<Object> exports);
  static Local<Object> Create(Statement* stmt);
  static NAN_METHOD(Run);
  static NAN_METHOD(Get);
  static NAN_METHOD(All);
  static NAN_METHOD(Reset);
  static NAN_METHOD(Finalize);
  static NAN_GETTER(GetSource);
  static NAN_GETTER(GetReader);

  static Nan::Persistent<FunctionTemplate> constructorTemplate;
  static Nan::Persistent<Function> constructor;

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
  static void bindValue(Statement* stmt, int index, Local<Value> val);

  // Convert row to JS object
  static Local<Object> rowToObject(Statement* stmt);
};

Nan::Persistent<FunctionTemplate> StatementWrap::constructorTemplate;
Nan::Persistent<Function> StatementWrap::constructor;

// ============================================
// Database Implementation
// ============================================

void DatabaseWrap::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Database").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Methods
  Nan::SetPrototypeMethod(tpl, "exec", Exec);
  Nan::SetPrototypeMethod(tpl, "prepare", Prepare);
  Nan::SetPrototypeMethod(tpl, "close", Close);

  // Accessors
  Nan::SetAccessor(tpl->InstanceTemplate(),
    Nan::New("open").ToLocalChecked(), GetOpen);
  Nan::SetAccessor(tpl->InstanceTemplate(),
    Nan::New("path").ToLocalChecked(), GetPath);
  Nan::SetAccessor(tpl->InstanceTemplate(),
    Nan::New("inTransaction").ToLocalChecked(), GetInTransaction);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("Database").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(DatabaseWrap::New) {
  if (!info.IsConstructCall()) {
    Nan::ThrowError("Use 'new' to create Database");
    return;
  }

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Path must be a string");
    return;
  }

  Nan::Utf8String path(info[0]);
  bool readonly = false;

  if (info.Length() >= 2 && info[1]->IsObject()) {
    Local<Object> opts = info[1].As<Object>();
    Local<Value> roVal = opts->Get(Nan::New("readonly").ToLocalChecked());
    if (roVal->IsBoolean()) {
      readonly = roVal->BooleanValue();
    }
  }

  try {
    DatabaseWrap* wrap = new DatabaseWrap();
    wrap->db_ = new Database(*path, readonly);
    wrap->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(DatabaseWrap::Exec) {
  DatabaseWrap* wrap = Nan::ObjectWrap::Unwrap<DatabaseWrap>(info.Holder());

  if (!wrap->db_ || !wrap->db_->isOpen()) {
    Nan::ThrowError("Database is closed");
    return;
  }

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("SQL must be a string");
    return;
  }

  Nan::Utf8String sql(info[0]);

  try {
    wrap->db_->exec(*sql);
    info.GetReturnValue().Set(info.Holder());
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(DatabaseWrap::Prepare) {
  DatabaseWrap* wrap = Nan::ObjectWrap::Unwrap<DatabaseWrap>(info.Holder());

  if (!wrap->db_ || !wrap->db_->isOpen()) {
    Nan::ThrowError("Database is closed");
    return;
  }

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("SQL must be a string");
    return;
  }

  Nan::Utf8String sql(info[0]);

  try {
    Statement* stmt = new Statement(wrap->db_, *sql);
    Local<Object> stmtObj = StatementWrap::Create(stmt);
    info.GetReturnValue().Set(stmtObj);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(DatabaseWrap::Close) {
  DatabaseWrap* wrap = Nan::ObjectWrap::Unwrap<DatabaseWrap>(info.Holder());

  if (wrap->db_) {
    wrap->db_->close();
  }
}

NAN_GETTER(DatabaseWrap::GetOpen) {
  DatabaseWrap* wrap = Nan::ObjectWrap::Unwrap<DatabaseWrap>(info.Holder());
  bool isOpen = wrap->db_ && wrap->db_->isOpen();
  info.GetReturnValue().Set(Nan::New<Boolean>(isOpen));
}

NAN_GETTER(DatabaseWrap::GetPath) {
  DatabaseWrap* wrap = Nan::ObjectWrap::Unwrap<DatabaseWrap>(info.Holder());
  if (wrap->db_) {
    info.GetReturnValue().Set(
      Nan::New<String>(wrap->db_->path().c_str()).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_GETTER(DatabaseWrap::GetInTransaction) {
  DatabaseWrap* wrap = Nan::ObjectWrap::Unwrap<DatabaseWrap>(info.Holder());
  bool inTx = wrap->db_ && wrap->db_->inTransaction();
  info.GetReturnValue().Set(Nan::New<Boolean>(inTx));
}

// ============================================
// Statement Implementation
// ============================================

void StatementWrap::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>();
  tpl->SetClassName(Nan::New("Statement").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "run", Run);
  Nan::SetPrototypeMethod(tpl, "get", Get);
  Nan::SetPrototypeMethod(tpl, "all", All);
  Nan::SetPrototypeMethod(tpl, "reset", Reset);
  Nan::SetPrototypeMethod(tpl, "finalize", Finalize);

  Nan::SetAccessor(tpl->InstanceTemplate(),
    Nan::New("source").ToLocalChecked(), GetSource);
  Nan::SetAccessor(tpl->InstanceTemplate(),
    Nan::New("reader").ToLocalChecked(), GetReader);

  constructorTemplate.Reset(tpl);
  constructor.Reset(tpl->GetFunction());
}

Local<Object> StatementWrap::Create(Statement* stmt) {
  Nan::EscapableHandleScope scope;

  Local<Function> cons = Nan::New(constructor);
  Local<Object> instance = Nan::NewInstance(cons).ToLocalChecked();

  StatementWrap* wrap = new StatementWrap();
  wrap->stmt_ = stmt;
  wrap->Wrap(instance);

  return scope.Escape(instance);
}

void StatementWrap::bindValue(Statement* stmt, int index, Local<Value> val) {
  if (val->IsNull() || val->IsUndefined()) {
    stmt->bindNull(index);
  }
  else if (val->IsBoolean()) {
    stmt->bindInt(index, val->BooleanValue() ? 1 : 0);
  }
  else if (val->IsInt32()) {
    stmt->bindInt(index, Nan::To<int32_t>(val).FromJust());
  }
  else if (val->IsNumber()) {
    double num = Nan::To<double>(val).FromJust();
    // Check if it's an integer that fits in int64
    if (num == static_cast<double>(static_cast<int64_t>(num))) {
      stmt->bindInt64(index, static_cast<int64_t>(num));
    } else {
      stmt->bindDouble(index, num);
    }
  }
  else if (val->IsString()) {
    Nan::Utf8String str(val);
    stmt->bindText(index, *str);
  }
  else if (node::Buffer::HasInstance(val)) {
    Local<Object> buf = val.As<Object>();
    stmt->bindBlob(index, node::Buffer::Data(buf), node::Buffer::Length(buf));
  }
  else {
    stmt->bindNull(index);
  }
}

Local<Object> StatementWrap::rowToObject(Statement* stmt) {
  Nan::EscapableHandleScope scope;

  Local<Object> row = Nan::New<Object>();
  int colCount = stmt->columnCount();

  for (int i = 0; i < colCount; i++) {
    std::string name = stmt->columnName(i);
    Local<String> key = Nan::New<String>(name.c_str()).ToLocalChecked();

    int type = stmt->columnType(i);
    Local<Value> value;

    switch (type) {
      case SQLITE_INTEGER:
        value = Nan::New<Number>(static_cast<double>(stmt->getInt64(i)));
        break;

      case SQLITE_FLOAT:
        value = Nan::New<Number>(stmt->getDouble(i));
        break;

      case SQLITE_TEXT:
        value = Nan::New<String>(stmt->getText(i).c_str()).ToLocalChecked();
        break;

      case SQLITE_BLOB: {
        int size;
        const void* data = stmt->getBlob(i, &size);
        value = Nan::CopyBuffer(static_cast<const char*>(data), size).ToLocalChecked();
        break;
      }

      case SQLITE_NULL:
      default:
        value = Nan::Null();
        break;
    }

    row->Set(key, value);
  }

  return scope.Escape(row);
}

NAN_METHOD(StatementWrap::Run) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  if (!wrap->stmt_ || !wrap->stmt_->isValid()) {
    Nan::ThrowError("Statement has been finalized");
    return;
  }

  try {
    wrap->stmt_->reset();
    wrap->stmt_->clearBindings();

    // Bind parameters
    for (int i = 0; i < info.Length(); i++) {
      bindValue(wrap->stmt_, i + 1, info[i]);
    }

    // Execute
    wrap->stmt_->step();

    // Return result object
    Local<Object> result = Nan::New<Object>();
    result->Set(Nan::New("changes").ToLocalChecked(),
                Nan::New<Integer>(wrap->stmt_->changes()));
    result->Set(Nan::New("lastInsertRowid").ToLocalChecked(),
                Nan::New<Number>(static_cast<double>(wrap->stmt_->lastInsertRowid())));

    info.GetReturnValue().Set(result);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(StatementWrap::Get) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  if (!wrap->stmt_ || !wrap->stmt_->isValid()) {
    Nan::ThrowError("Statement has been finalized");
    return;
  }

  try {
    wrap->stmt_->reset();
    wrap->stmt_->clearBindings();

    // Bind parameters
    for (int i = 0; i < info.Length(); i++) {
      bindValue(wrap->stmt_, i + 1, info[i]);
    }

    // Get first row
    if (wrap->stmt_->step()) {
      info.GetReturnValue().Set(rowToObject(wrap->stmt_));
    } else {
      info.GetReturnValue().Set(Nan::Undefined());
    }
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(StatementWrap::All) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  if (!wrap->stmt_ || !wrap->stmt_->isValid()) {
    Nan::ThrowError("Statement has been finalized");
    return;
  }

  try {
    wrap->stmt_->reset();
    wrap->stmt_->clearBindings();

    // Bind parameters
    for (int i = 0; i < info.Length(); i++) {
      bindValue(wrap->stmt_, i + 1, info[i]);
    }

    // Get all rows
    Local<Array> rows = Nan::New<Array>();
    uint32_t index = 0;

    while (wrap->stmt_->step()) {
      rows->Set(index++, rowToObject(wrap->stmt_));
    }

    info.GetReturnValue().Set(rows);
  } catch (const std::exception& e) {
    Nan::ThrowError(e.what());
  }
}

NAN_METHOD(StatementWrap::Reset) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  if (wrap->stmt_ && wrap->stmt_->isValid()) {
    wrap->stmt_->reset();
  }
}

NAN_METHOD(StatementWrap::Finalize) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  if (wrap->stmt_) {
    wrap->stmt_->finalize();
  }
}

NAN_GETTER(StatementWrap::GetSource) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  if (wrap->stmt_) {
    info.GetReturnValue().Set(
      Nan::New<String>(wrap->stmt_->source().c_str()).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_GETTER(StatementWrap::GetReader) {
  StatementWrap* wrap = Nan::ObjectWrap::Unwrap<StatementWrap>(info.Holder());

  bool isReader = wrap->stmt_ && wrap->stmt_->isReader();
  info.GetReturnValue().Set(Nan::New<Boolean>(isReader));
}

// ============================================
// Module Initialization
// ============================================

void InitSQLite3(Local<Object> exports) {
  Local<Object> sqlite3 = Nan::New<Object>();

  DatabaseWrap::Init(sqlite3);
  StatementWrap::Init(sqlite3);

  exports->Set(Nan::New("sqlite3").ToLocalChecked(), sqlite3);
}
