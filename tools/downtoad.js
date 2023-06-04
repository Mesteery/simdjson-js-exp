import { WASI } from 'node:wasi';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';

const nodeTarget = 'v20.0.0';

const wasi = new WASI({
  args: [, nodeTarget],
  version: 'preview1',
  preopens: {
    './': fileURLToPath(new URL('..', import.meta.url)),
  },
});

const wasm = await WebAssembly.compile(await readFile(new URL('../downtoad/downtoad.wasm', import.meta.url)));
const instance = await WebAssembly.instantiate(wasm, wasi.getImportObject());

wasi.start(instance);
