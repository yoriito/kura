---
title: "Yoriito VISS API Reference for Kura"
description: "API reference for the Yoriito VISS API for Kura"
---

# Yoriito VISS API Reference for Kura

Kura C++ Client implements the Yoriito VISS Consumer/Producer API for Kura.

These APIs are the reference implementation of the Yoriito VISS Consumer/Producer API.

For the interface definitions of Yoriito VISS Consumer/Producer API,<br>
please refer to the [Yoriito Vehicle API](https://github.com/yoriito/vehicle-api.git).

## Modules

| Module | Description |
|--------|-------------|
|[common](common/index.html)| The common parts of Yoriito VISS API. |
|[consumer](consumer/index.html)| Yoriito VISS Consumer API. |
|[producer](producer/index.html)| Yoriito VISS Producer API. |

## How to generate the documentation

To generate the documentation only, run the following commands under `client/cc`:

```bash
cmake -S . -B build -DBUILD_COMMON=OFF -DBUILD_CONSUMER=OFF -DBUILD_PRODUCER=OFF -DENABLE_DOCS=ON
cmake --build build --target docs
```
