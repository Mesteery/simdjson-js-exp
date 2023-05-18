1. [parser::parse](simdjson/include/simdjson/dom/parser-inl.h#L138)
2. [parser::parse_into_document](simdjson/include/simdjson/dom/parser-inl.h#L104)

```cpp
inline simdjson_result<element> parser::parse_into_document(document& provided_doc, const uint8_t *buf, size_t len, bool realloc_if_needed) & noexcept {
  // Important: we need to ensure that document has enough capacity.
  // Important: It is possible that provided_doc is actually the internal 'doc' within the parser!!!
  error_code _error = ensure_capacity(provided_doc, len);
  if (_error) { return _error; }
  if (realloc_if_needed) {
    // Make sure we have enough capacity to copy len bytes
    if (!loaded_bytes || _loaded_bytes_capacity < len) {
      loaded_bytes.reset( internal::allocate_padded_buffer(len) );
      if (!loaded_bytes) {
        return MEMALLOC;
      }
      _loaded_bytes_capacity = len;
    }
    std::memcpy(static_cast<void *>(loaded_bytes.get()), buf, len);
  }
  _error = implementation->parse(realloc_if_needed ? reinterpret_cast<const uint8_t*>(loaded_bytes.get()): buf, len, provided_doc);

  if (_error) { return _error; }

  return provided_doc.root();
}
```

3. `implementation->parse`
4. arm64 dom impl `parse`
5. `stage1`
6. [json_structural_indexer::index](simdjson/src/generic/stage1/json_structural_indexer.h#L197)

```cpp
template<size_t STEP_SIZE>
error_code json_structural_indexer::index(const uint8_t *buf, size_t len, dom_parser_implementation &parser, stage1_mode partial) noexcept {
  if (simdjson_unlikely(len > parser.capacity())) { return CAPACITY; }
  // We guard the rest of the code so that we can assume that len > 0 throughout.
  if (len == 0) { return EMPTY; }
  buf_block_reader<STEP_SIZE> reader(buf, len);
  json_structural_indexer indexer(parser.structural_indexes.get());

  // Read all but the last block
  while (reader.has_full_block()) {
    indexer.step<STEP_SIZE>(reader.full_block(), reader);
  }
  // Take care of the last block (will always be there unless file is empty which is
  // not supposed to happen.)
  uint8_t block[STEP_SIZE];
  if (simdjson_unlikely(reader.get_remainder(block) == 0)) { return UNEXPECTED_ERROR; }
  indexer.step<STEP_SIZE>(block, reader);
  return indexer.finish(parser, reader.block_index(), len, partial);
```
