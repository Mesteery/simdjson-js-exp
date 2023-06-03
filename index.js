import { readFile } from 'fs/promises';
import Benchmark from 'benchmark';
import { parseJson, findStructuralIndexes } from './simdjson.js';

const bdata = await readFile('./data/large-file.json');
const data = bdata.toString();

new Benchmark.Suite()
  //.add('napi', () => parseJson(bdata))
  .add('napi rs', () => findStructuralIndexes(bdata))
  //.add('JSON', () => JSON.parse(data))
  .on('cycle', (event) => console.log(event.target.toString()))
  .on('complete', function () {
    console.log(`Fastest is ${this.filter('fastest').map('name')}`);
    //console.log('x%f%', (this[0].hz / this[1].hz).toFixed(4) * 100);
  })
  .run({ async: true });
