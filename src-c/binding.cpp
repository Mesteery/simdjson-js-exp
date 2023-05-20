// hello.cc
#include <napi.h>

#include "simdjson.h"

namespace demo {

Napi::Value Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  const auto ab = info[0].As<Napi::Uint8Array>();
  std::unique_ptr<simdjson::internal::dom_parser_implementation> impl;
  simdjson::get_active_implementation()->create_dom_parser_implementation(
      ab.ElementLength(), 0, impl);
  impl->stage1(ab.Data(), ab.ElementLength(), simdjson::stage1_mode::regular);

  return Napi::Uint32Array::New(
      env, impl->n_structural_indexes,
      Napi::ArrayBuffer::New(env, impl->structural_indexes.release(),
                             4 * impl->n_structural_indexes),
      0);

  /*std::unique_ptr<v8::BackingStore> backing =
  v8::ArrayBuffer::NewBackingStore( impl->structural_indexes.release(),
      static_cast<size_t>(impl->n_structural_indexes) * 4,
      [](void*, size_t, void*) {}, nullptr);
  const auto ret = v8::ArrayBuffer::New(args.GetIsolate(), std::move(backing));

  args.GetReturnValue().Set(v8::Uint32Array::New(
      ret, 0, static_cast<size_t>(impl->n_structural_indexes)));*/
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  simdjson::get_active_implementation();
  exports.Set(Napi::String::New(env, "hello"),
              Napi::Function::New(env, Method));
  return exports;
}

NODE_API_MODULE(addon, Init)

}  // namespace demo
