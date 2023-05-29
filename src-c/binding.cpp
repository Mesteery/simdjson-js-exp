// hello.cc
#include <napi.h>

#include "simdjson.h"

#define TRY(EXPR)                                          \
  {                                                        \
    auto _err = (EXPR);                                    \
    if (_err) {                                            \
      Napi::Error::New(env, simdjson::error_message(_err)) \
          .ThrowAsJavaScriptException();                   \
      return env.Undefined();                              \
    }                                                      \
  }

#define THROW(ERROR_CODE)                                      \
  {                                                            \
    Napi::Error::New(env, simdjson::error_message(ERROR_CODE)) \
        .ThrowAsJavaScriptException();                         \
    return env.Undefined();                                    \
  }

constexpr uint32_t DEFAULT_CAPACITY = 1000 * 1000;

class JsonIndexer : public Napi::ObjectWrap<JsonIndexer> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(
        env, "JsonIndexer",
        {
            InstanceMethod<&JsonIndexer::FindStructuralIndexes>(
                "findStructuralIndexes"),
            InstanceMethod<&JsonIndexer::ParseKey>("parseKey"),
            InstanceMethod<&JsonIndexer::ParsePrimitive>("parsePrimitive"),
            InstanceMethod<&JsonIndexer::ParseRootPrimitive>(
                "parseRootPrimitive"),
        });
    Napi::FunctionReference* constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    exports.Set("JsonIndexer", func);
    env.SetInstanceData<Napi::FunctionReference>(constructor);
    return exports;
  }

  JsonIndexer(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<JsonIndexer>(info) {
    simdjson::get_active_implementation()->create_dom_parser_implementation(
        DEFAULT_CAPACITY, simdjson::DEFAULT_MAX_DEPTH, implementation);
    size_t string_capacity = SIMDJSON_ROUNDUP_N(
        5 * DEFAULT_CAPACITY / 3 + simdjson::SIMDJSON_PADDING, 64);
    string_buf.reset(new uint8_t[string_capacity]);
  }

 private:
  Napi::Value FindStructuralIndexes(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto ab = info[0].As<Napi::Uint8Array>();
    len = ab.ByteLength();

    if (implementation->capacity() < len) {
      size_t string_capacity =
          SIMDJSON_ROUNDUP_N(5 * len / 3 + simdjson::SIMDJSON_PADDING, 64);
      string_buf.reset(new uint8_t[string_capacity]);
      TRY(implementation->set_capacity(len));
    }

    buf = ab.Data();
    TRY(implementation->stage1(buf, len, simdjson::stage1_mode::regular));

    return Napi::Uint32Array::New(
        env, implementation->n_structural_indexes,
        Napi::ArrayBuffer::New(env, implementation->structural_indexes.get(),
                               4 * implementation->n_structural_indexes),
        0);

    /*std::unique_ptr<v8::BackingStore> backing =
    v8::ArrayBuffer::NewBackingStore( impl->structural_indexes.release(),
        static_cast<size_t>(impl->n_structural_indexes) * 4,
        [](void*, size_t, void*) {}, nullptr);
    const auto ret = v8::ArrayBuffer::New(args.GetIsolate(),
    std::move(backing));

    args.GetReturnValue().Set(v8::Uint32Array::New(
        ret, 0, static_cast<size_t>(impl->n_structural_indexes)));*/
  }

  inline Napi::Value ParseString(const uint8_t* value) {
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
  }

  std::unique_ptr<simdjson::internal::dom_parser_implementation> implementation;
  std::unique_ptr<uint8_t[]> string_buf{};
  const uint8_t* buf = nullptr;
  size_t len;
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  JsonIndexer::Init(env, exports);
  return exports;
}

NODE_API_MODULE(addon, Init)
