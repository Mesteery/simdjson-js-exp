import { WASI } from 'node:wasi';
import { argv, env } from 'node:process';
import { readFile } from 'node:fs/promises';

const wasi = new WASI({
  args: argv,
  env: { ...env, RUST_BACKTRACE: '1' },
  version: 'preview1',
  preopens: {
    '/': '../sandbox',
  },
});

const wasm = await WebAssembly.compile(
  await readFile(new URL('../downtoad/target/wasm32-wasi/debug/downtoad.wasm', import.meta.url)),
);
const instance = await WebAssembly.instantiate(wasm, wasi.getImportObject());

wasi.start(instance);
