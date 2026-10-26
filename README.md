# Kura 

Kura is a platform for vehicle signal processing, data management, and client/server communication, centered around the Vehicle Signal Specification (VSS).

## Features

- Written in Rust
- Provides a lightweight gRPC server for VSS data
- Flexible database backend for VSS signal tree
- Easy integration with external applications via RPC

## Architecture

Kura follows Clean Architecture principles:
- Layered structure with clear separation of concerns
- Interfaces between layers defined as traits and structs
- Synchronous communication between layers
- Multithreaded runtime using tokio (Rust)
- Extensible via protocol buffers and gRPC

## Supported APIs

- gRPC endpoints for VSS data access and manipulation
- Protocol buffer definitions for custom extensions
- WebSocket and REST support (planned/experimental)

## Repository Structure

- **kura/**: Rust gRPC server and core database
- **client/**: C++ and Python client libraries
- **docs/**: Documentation and design materials
- **external/**: External dependencies and proto files
- **proto/**: Protocol buffer definitions
- **scripts/**: Utility scripts for code generation and setup

## Installation

### Prerequisites
- Rust toolchain (for server)
- Python (for client tools)

### Build and Run
```sh
cd kura
cargo build --release
cargo run --release -- --config-file-path config.yml
```

## Usage

- Use client libraries in `client/cc` or `client/vshot` to connect to the server

## Documentation

See the `docs/` directory for software design, architecture, and usage guides.

## Versioning

This project follows [Semantic Versioning](https://semver.org/).

The version of vehicle-api is managed independently from the overall Yoriito version.

## Quality Assessment
 
This repository contains quality assessment results for this project.
Please refer to the [quality assessment page](./quality/quality-assessment.md).

The results are provided for transparency and reference purposes and do not constitute any warranty or guarantee of quality.

### Contents
 
- Unit Test
- Coding Style
- Code Metrics
- Static Analysis
- Dynamic Analysis
- SCA
- SBOM

### Notes
 
Some files in this directory may be generated automatically by CI tools.

