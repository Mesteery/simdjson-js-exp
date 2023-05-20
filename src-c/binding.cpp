// hello.cc
#include <node.h>

#include "simdjson.h"

namespace demo {

void Method(const v8::FunctionCallbackInfo<v8::Value>& args) {
  const auto ab = args[0].As<v8::ArrayBuffer>();
  const uint8_t* buf = reinterpret_cast<const uint8_t*>(ab->Data());
  std::unique_ptr<simdjson::internal::dom_parser_implementation> impl;
  impl->stage1(buf, ab->ByteLength(), simdjson::stage1_mode::regular);

  /*std::unique_ptr<v8::BackingStore> backing =
  v8::ArrayBuffer::NewBackingStore( impl->structural_indexes.release(),
      static_cast<size_t>(impl->n_structural_indexes),
      [](void*, size_t, void*) {}, nullptr);
  const auto ret = v8::ArrayBuffer::New(args.GetIsolate(), std::move(backing));

  args.GetReturnValue().Set(v8::Uint32Array::New(
      ret, 0, static_cast<size_t>(impl->n_structural_indexes)));*/
}

void Initialize(v8::Local<v8::Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

}  // namespace demo
