#[cfg(not(target_arch = "wasm32"))]
#[macro_use]
extern crate napi_derive;
use napi::bindgen_prelude::*;

use simd_json::{Deserializer, Error as SimdJsonError};

#[cfg(target_arch = "wasm32")]
#[no_mangle]
pub fn find_structural_indexes() -> () {
  let _ = unsafe { Deserializer::find_structural_bits(INPUT) };
}

#[cfg(not(target_arch = "wasm32"))]
#[napi]
pub fn find_structural_indexes(input: Uint8Array) -> Result<Uint32Array> {
  match unsafe { Deserializer::find_structural_bits(&input) } {
    Ok(ret) => Ok(Uint32Array::new(ret)),
    Err(err) => Err(Error::new(Status::GenericFailure, SimdJsonError::generic(err).to_string())),
  }
}
