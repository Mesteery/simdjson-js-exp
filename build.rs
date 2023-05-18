fn main() {
 #[cfg(not(target_arch = "wasm32"))]
  extern crate napi_build;
  napi_build::setup();
}
