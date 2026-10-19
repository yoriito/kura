// SPDX-License-Identifier: Apache-2.0
//
// Copyright 2026 JASPAR
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Generate client-side proto types for the gRPC fuzz harness.
    // Set YORIITO_VISS_PROTO_DIR to override the default proto search path.
    let yoriito_viss_proto_dir = std::env::var("YORIITO_VISS_PROTO_DIR")
        .unwrap_or_else(|_| "../../external/yoriito-viss/proto".to_string());
    println!("cargo:rerun-if-env-changed=YORIITO_VISS_PROTO_DIR");

    let protos = vec![
        format!("{yoriito_viss_proto_dir}/yoriito/viss/v1/types/error.proto"),
        format!("{yoriito_viss_proto_dir}/yoriito/viss/v1/types/filter.proto"),
        format!("{yoriito_viss_proto_dir}/yoriito/viss/v1/types/value.proto"),
        format!("{yoriito_viss_proto_dir}/yoriito/viss/v1/common.proto"),
        format!("{yoriito_viss_proto_dir}/yoriito/viss/v1/consumer.proto"),
        format!("{yoriito_viss_proto_dir}/yoriito/viss/v1/producer.proto"),
    ];

    tonic_prost_build::configure()
        .build_client(true)
        .build_server(false)
        .protoc_arg("--experimental_allow_proto3_optional")
        .compile_protos(&protos, std::slice::from_ref(&yoriito_viss_proto_dir))?;

    Ok(())
}
