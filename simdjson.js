import { dlopen } from 'process';
import { constants } from 'os';
import { fileURLToPath } from 'url';

const binding = { exports: {} };
dlopen(binding, fileURLToPath(new URL('./build/Release/addon.node', import.meta.url)), constants.dlopen.RTLD_NOW);
const { JsonIndexer } = binding.exports;

const BRACE_OPEN = 123; // {
const BRACE_CLOSE = 125; // }
const BRACKET_OPEN = 91; // [
const BRACKET_CLOSE = 93; // ]
const COMMA = 44; // ,
const COLON = 58; // :
const QUOTE = 34; // "

const OBJECT_BEGIN = 0;
const OBJECT_FIELD = 1;
const OBJECT_CONTINUE = 2;
const ARRAY_BEGIN = 3;
const ARRAY_VALUE = 4;
const ARRAY_CONTINUE = 5;

const parser = new JsonIndexer();
const intermediateData = new Array(1025);
const intermediateKeys = new Array(1025);

export function parseJson(input) {
  const structuralIndexes = parser.findStructuralIndexes(input);
  let nextStructural = 0;
  let state = OBJECT_BEGIN;
  let depth = 0;

  switch (input[structuralIndexes[nextStructural++]]) {
    case BRACE_OPEN: {
      if (input[structuralIndexes[structuralIndexes.length - 1]] != BRACE_CLOSE)
        throw new Error('starting brace unmatched');
      if (input[structuralIndexes[1]] == BRACE_CLOSE) return {};
      //state = object_begin
      break;
    }
    case BRACKET_OPEN: {
      if (input[structuralIndexes[structuralIndexes.length - 1]] != BRACKET_CLOSE)
        throw new Error('starting bracket unmatched');
      if (input[structuralIndexes[1]] == BRACKET_CLOSE) return new Array(0);
      state = ARRAY_BEGIN;
      break;
    }
    default:
      return parser.parseRootPrimitive();
  }

  loop: for (;;) {
    switch (state) {
      case OBJECT_BEGIN: {
        const key = structuralIndexes[nextStructural++];
        if (input[key] != QUOTE) throw new Error('object key must be a string');
        intermediateData[++depth] = {};
        intermediateKeys[depth] = parser.parseKey(key);
      }

      case OBJECT_FIELD: {
        if (input[structuralIndexes[nextStructural++]] != COLON)
          throw new Error('object field must be followed by a colon');

        const value = structuralIndexes[nextStructural++];
        switch (input[value]) {
          case BRACE_OPEN: {
            if (input[structuralIndexes[nextStructural]] == BRACE_CLOSE) {
              ++nextStructural;
              intermediateData[depth][intermediateKeys[depth]] = {};
              break;
            }
            state = OBJECT_BEGIN;
            continue loop;
          }
          case BRACKET_OPEN: {
            if (input[structuralIndexes[nextStructural]] == BRACKET_CLOSE) {
              ++nextStructural;
              intermediateData[depth][intermediateKeys[depth]] = [];
              break;
            }
            state = ARRAY_BEGIN;
            continue loop;
          }
          default:
            intermediateData[depth][intermediateKeys[depth]] = parser.parsePrimitive(value);
        }
      }

      case OBJECT_CONTINUE:
        switch (input[structuralIndexes[nextStructural++]]) {
          case COMMA: {
            const key = structuralIndexes[nextStructural++];
            if (input[key] != QUOTE) throw new Error('object key must be a string');
            intermediateKeys[depth] = parser.parseKey(key);
            state = OBJECT_FIELD;
            continue loop;
          }
          case BRACE_CLOSE:
            // scope end
            if (--depth == 0) break loop;
            if (Array.isArray(intermediateData[depth])) {
              intermediateData[depth].push(intermediateData[depth + 1]);
              state = ARRAY_CONTINUE;
              continue loop;
            }
            intermediateData[depth][intermediateKeys[depth]] = intermediateData[depth + 1];
            state = OBJECT_CONTINUE;
            continue loop;
          default:
            throw new Error('object field must be followed by a comma or a closing brace');
        }

      case ARRAY_BEGIN:
        intermediateData[++depth] = [];

      case ARRAY_VALUE: {
        const value = structuralIndexes[nextStructural++];
        switch (input[value]) {
          case BRACE_OPEN:
            if (input[structuralIndexes[nextStructural]] == BRACE_CLOSE) {
              ++nextStructural;
              intermediateData[depth].push({});
              break;
            }
            state = OBJECT_BEGIN;
            continue loop;
          case BRACKET_OPEN:
            if (input[structuralIndexes[nextStructural]] == BRACKET_CLOSE) {
              ++nextStructural;
              intermediateData[depth].push(new Array(0));
              break;
            }
            state = ARRAY_BEGIN;
            continue loop;
          default:
            intermediateData[depth].push(parser.parsePrimitive(value));
        }
      }

      case ARRAY_CONTINUE:
        switch (input[structuralIndexes[nextStructural++]]) {
          case COMMA:
            state = ARRAY_VALUE;
            continue loop;
          case BRACKET_CLOSE:
            // scope end
            if (--depth == 0) break loop;
            if (Array.isArray(intermediateData[depth])) {
              intermediateData[depth].push(intermediateData[depth + 1]);
              state = ARRAY_CONTINUE;
              continue loop;
            }
            intermediateData[depth][intermediateKeys[depth]] = intermediateData[depth + 1];
            state = OBJECT_CONTINUE;
            continue loop;
          default:
            throw new Error('array value must be followed by a comma or a closing bracket');
        }
    }
  }

  return intermediateData[1];
}
