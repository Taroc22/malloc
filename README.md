## Simple custom implementation of C's malloc().

### Improvements needed to make it usable:

- Change sbrk to mmap (Unix) / VirtualAlloc (Windows)
- Coalescing in both directions (currently only next block)
- Different search algorithms (Segregated Free Lists) to reduce O() complexity
- API completeness (realloc,calloc,etc)
- Header‑Magic/Canary, Double-Free, Safe unlink, Overflow guards
- Thread safety (e.g. thread local block cache)
- increase standard payload (e.g. 16B)
- min_split to prevent fragmentation


