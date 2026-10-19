# Client Test Environment

This directory contains the test environment for the client integrated with the Kura server.

## Directory

```
test_env/
├── vss/
│   └── test.vspec         # Test VSS specification file
├── vss.json               # Yoriito Vehicle Signal Specification
├── server.json            # Kura server configuration file
├── config.yml             # Kura runtime configuration file
└── start_kura_test.sh     # Script to execute the complete test flow
```

## Test Execution

### Starting a Docker container
```
docker compose -f .devcontainer/docker-compose.yml -f .devcontainer/docker-compose.auto.yml up -d
```

### Build Kura
```
docker compose -f .devcontainer/docker-compose.yml -f .devcontainer/docker-compose.auto.yml exec dev bash -lc "source ~/.cargo/env && cd /work/kura && cargo build --release"
```

### Build Client
```
docker compose -f .devcontainer/docker-compose.yml -f .devcontainer/docker-compose.auto.yml exec dev bash -lc "cd /work/client/cc && rm -rf build && mkdir build && cd build && cmake -DENABLE_TESTS=ON .. && make -j$(nproc)"
```

### Test
```
docker compose -f .devcontainer/docker-compose.yml -f .devcontainer/docker-compose.auto.yml exec dev bash -lc "chmod +x /work/client/cc/test_env/*.sh && cd /work/client/cc/test_env && ./start_kura_test.sh"
```
