#include <node.h>
#include <node_object_wrap.h>

#include "simdjson.h"

using namespace v8;

#define TRY(EXPR)         \
  {                       \
    auto _err = (EXPR);   \
    if (_err) THROW(_err) \
  }

#define THROW(ERROR_CODE)                                                 \
  {                                                                       \
    isolate->ThrowError(                                                  \
        String::NewFromUtf8(isolate, simdjson::error_message(ERROR_CODE)) \
            .ToLocalChecked());                                           \
    return;                                                               \
  }

constexpr uint32_t DEFAULT_CAPACITY = 1000 * 1000;
constexpr uint32_t DEFAULT_MAX_DEPTH = 64;

class JsonIndexer : public node::ObjectWrap {
 public:
  static void Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<ObjectTemplate> addon_data_tpl = ObjectTemplate::New(isolate);
    addon_data_tpl->SetInternalFieldCount(1);
    Local<Object> addon_data =
        addon_data_tpl->NewInstance(context).ToLocalChecked();

    Local<FunctionTemplate> tpl =
        FunctionTemplate::New(isolate, New, addon_data);

    tpl->SetClassName(
        String::NewFromUtf8(isolate, "JsonIndexer").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "findStructuralIndexes", FindStructuralIndexes);

    Local<Function> constructor = tpl->GetFunction(context).ToLocalChecked();
    addon_data->SetInternalField(0, constructor);
    exports
        ->Set(context,
              String::NewFromUtf8(isolate, "JsonIndexer").ToLocalChecked(),
              constructor)
        .FromJust();
  }

 private:
  JsonIndexer() {
    simdjson::get_active_implementation()->create_dom_parser_implementation(
        DEFAULT_CAPACITY, DEFAULT_MAX_DEPTH, implementation);
    size_t string_capacity =
        SIMDJSON_ROUNDUP_N(DEFAULT_CAPACITY + simdjson::SIMDJSON_PADDING, 64);
    string_buf.reset(new uint8_t[string_capacity]);
  }

  ~JsonIndexer() {}

  static void New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.IsConstructCall()) {
      JsonIndexer* obj = new JsonIndexer();
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      Local<Function> cons =
          args.Data().As<Object>()->GetInternalField(0).As<Function>();
      Local<Object> result = cons->NewInstance(context).ToLocalChecked();
      args.GetReturnValue().Set(result);
    }
  }

  static void FindStructuralIndexes(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    JsonIndexer* obj = ObjectWrap::Unwrap<JsonIndexer>(args.Holder());

    auto ab = args[0].As<Uint8Array>()->Buffer();
    obj->len = ab->ByteLength();

    if (obj->implementation->capacity() < obj->len) {
      size_t string_capacity =
          SIMDJSON_ROUNDUP_N(obj->len + simdjson::SIMDJSON_PADDING, 64);
      obj->string_buf.reset(new uint8_t[string_capacity]);
      TRY(obj->implementation->set_capacity(obj->len));
    }

    obj->buf = static_cast<const uint8_t*>(ab->Data());
    TRY(obj->implementation->stage1(obj->buf, obj->len,
                                    simdjson::stage1_mode::regular));

    auto backing = ArrayBuffer::NewBackingStore(
        obj->implementation->structural_indexes.get(),
        4 * obj->implementation->n_structural_indexes,
        [](void*, size_t, void*) {}, nullptr);

    args.GetReturnValue().Set(
        Uint32Array::New(ArrayBuffer::New(isolate, std::move(backing)), 0,
                         obj->implementation->n_structural_indexes));
  }

  /*inline Napi::Value ParseString(const uint8_t* value) {
    auto env = Env();

    auto dst = string_buf.get();
    auto dstend = implementation->parse_string(value + 1, dst, false);

    if (dstend == nullptr) {
      THROW(simdjson::STRING_ERROR);
    }

    auto str = Napi::String::New(env, reinterpret_cast<const char*>(dst),
                                 dstend - dst);

    return str;
  }

  Napi::Value ParseKey(const Napi::CallbackInfo& info) {
    return ParseString(&buf[info[0].As<Napi::Number>().Uint32Value()]);
  }

  Napi::Value ParsePrimitive(const Napi::CallbackInfo& info) {
    auto env = Env();

    uint32_t structural_index = info[0].As<Napi::Number>().Uint32Value();

    auto value = &buf[structural_index];
    switch (*value) {
      case '"':
        return ParseString(value);
      case 't':
        if (simdjson::builtin::atomparsing::is_valid_true_atom(value)) {
          return Napi::Boolean::New(env, true);
        }
        THROW(simdjson::T_ATOM_ERROR);
      case 'f':
        if (simdjson::builtin::atomparsing::is_valid_false_atom(value)) {
          return Napi::Boolean::New(env, false);
        }
        THROW(simdjson::F_ATOM_ERROR);
      case 'n':
        if (simdjson::builtin::atomparsing::is_valid_null_atom(value)) {
          return env.Null();
        }
        THROW(simdjson::N_ATOM_ERROR);
      case '-':  // clang-format off
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':  // clang-format on
      {
        auto ret = simdjson::builtin::numberparsing::parse_double(value);
        TRY(ret.error());
        return Napi::Number::New(env, ret.value_unsafe());
      }
      default:
        THROW(simdjson::TAPE_ERROR);
    }
  }

  Napi::Value ParseRootPrimitive(const Napi::CallbackInfo& info) {
    auto env = Env();

    auto value = &buf[implementation->structural_indexes[0]];
    size_t remaining_len = len - implementation->structural_indexes[0];
    switch (*value) {
      case '"':
        return ParseString(value);
      case 't':
        if (simdjson::builtin::atomparsing::is_valid_true_atom(value,
                                                               remaining_len)) {
          return Napi::Boolean::New(env, true);
        }
        THROW(simdjson::T_ATOM_ERROR);
      case 'f':
        if (simdjson::builtin::atomparsing::is_valid_false_atom(
                value, remaining_len)) {
          return Napi::Boolean::New(env, false);
        }
        THROW(simdjson::F_ATOM_ERROR);
      case 'n':
        if (simdjson::builtin::atomparsing::is_valid_null_atom(value,
                                                               remaining_len)) {
          return env.Null();
        }
        THROW(simdjson::N_ATOM_ERROR);
      case '-':  // clang-format off
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':  // clang-format on
      {
        auto ret = simdjson::builtin::numberparsing::parse_double(value);
        TRY(ret.error());
        return Napi::Number::New(env, ret.value_unsafe());
      }
      default:
        THROW(simdjson::TAPE_ERROR);
    }
  }*/

  std::unique_ptr<simdjson::internal::dom_parser_implementation> implementation;
  std::unique_ptr<uint8_t[]> string_buf{};
  const uint8_t* buf = nullptr;
  size_t len;
};

void Init(Local<Object> exports) { JsonIndexer::Init(exports); }

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
