# C++ Client Fuzzing

All harnesses use AFL++. The AFL++ compiler backend is selected via `AFL_COMPILER`:

| `CMAKE_CXX_COMPILER` | Notes |
|---|---|
| `afl-g++` (default)| GCC plugin instrumentation |
| `afl-clang-fast++` | LTO instrumentation, faster feedback |

## Build and run with GCC (default)

```sh
cd client/cc
mkdir build_fuzz_gcc && cd build_fuzz_gcc
cmake -DCMAKE_CXX_COMPILER=afl-g++ \
      -DENABLE_FUZZ_TESTS=ON \
      -DENABLE_ASAN=ON \
      -DENABLE_UBSAN=ON \
      ..
make fuzz_convert_value_type fuzz_convert_get_response

afl-fuzz -i ../test_fuzz/corpus/value_type/ \
         -o ../test_fuzz/findings_convert_value_type/ \
         -- ./fuzz_convert_value_type
```

## Build and run with Clang

```sh
cd client/cc
mkdir build_fuzz && cd build_fuzz
cmake -DCMAKE_CXX_COMPILER=afl-clang-fast++ \
      -DENABLE_FUZZ_TESTS=ON \
      -DENABLE_ASAN=ON \
      -DENABLE_UBSAN=ON \
      ..
make fuzz_convert_value_type fuzz_convert_get_response

afl-fuzz -i ../test_fuzz/corpus/value_type/ \
         -o ../test_fuzz/findings_convert_value_type/ \
         -- ./fuzz_convert_value_type

afl-fuzz -i ../test_fuzz/corpus/empty_value/ \
         -o ../test_fuzz/findings_convert_get_response/ \
         -- ./fuzz_convert_get_response
```


---
