# kura

kura is a Rust implementation of a gRPC server.

## Features

- Maintains a database with VSS (Vehicle Signal Specification) data structure.
- Provides read/write access to the database for external applications via RPC interfaces such as gRPC.

## Build

```sh
cd kura
cargo build --release
```

## Usage

```sh
cargo run --release -- --config-file-path config.yml
# if you want to see verbose log
cargo run --release -- --config-file-path config.yml --verbose
```

# Architecture

The project follows Clean Architecture principles:
- Each layer's interfaces are defined as traits and structs in the `interface` directory.
- Synchronous communication between layers (no mpsc channels are used, as they complicate synchronous processing and may reduce performance).

# Directory Structure

- `src/` : Main server source code
  - `main.rs` : Entry point
  - `common/` : Common definitions
  - `controller/` : Entry points for CLI and gRPC server
  - `entity/` : Entities such as database and signal models
  - `interactor/` : Business logic layer
  - `interface/` : Traits and interfaces
- `Cargo.toml` : Rust package definition
- `build.rs` : Build script for gRPC/Protobuf code generation
- `config.yml` : Example server configuration file

