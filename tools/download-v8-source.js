import { WASI } from 'node:wasi';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';

const nodeTarget = 'v20.0.0';

const wasi = new WASI({
  args: ['', nodeTarget],
  env: {},
  version: 'preview1',
  preopens: {
    './': fileURLToPath(new URL('..', import.meta.url)),
  },
});

const wasm = await WebAssembly.compile(await readFile(new URL('./download-cli.wasm', import.meta.url)));
const instance = await WebAssembly.instantiate(wasm, {
  ...wasi.getImportObject(),
  env: new Proxy(
    {},
    {
      get(target, prop) {
        console.log(prop);
        return console.log
      },
    },
  ),
});

wasi.start(instance);
