import { once } from 'events';
import { createReadStream, createWriteStream } from 'fs';
import { mkdir, writeFile } from 'fs/promises';
import { get } from 'https';
import { dirname, join } from 'path';
import { pipeline } from 'stream/promises';
import { extract as untar } from 'tar-stream';
import { createGunzip } from 'zlib';

await mkdir('node-v20.0.0/deps', { recursive: true });

const extract = untar();
extract.on('entry', async function (header, stream, next) {
  if (header.name.includes('/deps/v8/src') || header.name.includes('/deps/v8/include')) {
    if (header.type === 'directory') {
      await mkdir(header.name);
    } else {
      stream.pipe(createWriteStream(header.name));
    }
  }

  stream.resume();
  next();
});

//const [response] = await once(get('https://nodejs.org/dist/v20.0.0/node-v20.0.0.tar.gz'), 'response');
//await pipeline(response, createGunzip(), createWriteStream('node-v20.0.0.tar'));
await pipeline(createReadStream('node-v20.0.0.tar'), extract);
