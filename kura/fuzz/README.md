# kura fuzizing-test

## Rust Server Fuzzing (cargo-fuzz)

Requires nightly Rust and cargo-fuzz.

```sh
# Install cargo-fuzz if not already installed
cargo install cargo-fuzz --version 0.12.0 --locked
```

## 1. Verify library build

```sh
cd kura
cargo build
```

## 2. Verify fuzz target build

```sh
cd kura
cargo +nightly fuzz build
```

## 3. Run fuzz targets

Replace `<duration>` with the number of seconds to run each target (e.g. `30`).

```sh
cd kura
cargo +nightly fuzz run fuzz_value_type_conversion -- -max_total_time="<duration>"
cargo +nightly fuzz run fuzz_glob_path_filter -- -max_total_time="<duration>"
cargo +nightly fuzz run fuzz_mem_database_set -- -max_total_time="<duration>"
cargo +nightly fuzz run fuzz_get_condition_glob -- -max_total_time="<duration>"
cargo +nightly fuzz run fuzz_change_filter -- -max_total_time="<duration>"
cargo +nightly fuzz run fuzz_grpc_consumer -- -max_total_time="<duration>"
cargo +nightly fuzz run fuzz_grpc_producer -- -max_total_time="<duration>"
```

### 3.1 Run all fuzz targets

The script accepts an optional duration argument (seconds per target, default: 30).

```sh
cd kura
./fuzz/run_fuzz.sh          # 30 seconds per target (default)
./fuzz/run_fuzz.sh 60       # 60 seconds per target
```

## 4. Verify results

A fuzzing run passes if the command exits with code 0 and no crash artifacts are produced.
If a crash is found, cargo-fuzz saves the input to `fuzz/artifacts/<target>/` and exits with a non-zero code.

```sh
# Check for crash artifacts after a run
ls kura/fuzz/artifacts/
```
