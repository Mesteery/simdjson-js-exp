import { writeFile } from 'fs/promises';

async function download(fileName) {
  const file = await fetch(`https://github.com/simdjson/simdjson/releases/latest/download/${fileName}`).then((r) =>
    r.text(),
  );

  return writeFile(new URL(`src-c/${fileName}`, import.meta.url), file);
}

await download('simdjson.cpp');
await download('simdjson.h');
