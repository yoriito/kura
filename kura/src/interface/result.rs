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

use std::fmt::Display;

#[derive(
    Clone,
    Copy,
    Debug,
    PartialEq,
    Eq,
    Hash,
    PartialOrd,
    Ord,
    ::prost::Enumeration,
    strum_macros::Display,
)]
#[repr(i32)]
pub enum KuraErrorCode {
    /// Unspecified error code
    Unspecified = 0,
    /// error when the request is invalid or malformed
    BadRequest = 400,
    /// error when the request is unauthorized
    Unauthorized = 401,
    /// error when the request is forbidden
    Forbidden = 403,
    /// error when the requested node is not found
    NotFound = 404,
    /// error when the request is not acceptable
    RequestTimeout = 408,
    /// error when the server is overloaded or too many requests are made
    TooManyRequests = 429,
    /// error when the server encountered an internal error
    BadGateway = 502,
    /// error when the server is currently unavailable
    ServiceUnavailable = 503,
    /// error when the server did not receive a timely response from an upstream server
    GatewayTimeout = 504,
}

#[derive(
    Clone,
    Copy,
    Debug,
    PartialEq,
    Eq,
    Hash,
    PartialOrd,
    Ord,
    ::prost::Enumeration,
    strum_macros::Display,
)]
#[repr(i32)]
pub enum KuraErrorReason {
    /// Unspecified reason for the error
    Unspecified = 0,
    /// Bad request, typically due to malformed request syntax
    BadRequest = 1,
    /// Invalid data provided in the request
    InvalidData = 2,
    /// Invalid token, such as an expired or malformed request id
    InvalidToken = 3,
    /// Forbidden request, such as when the user does not have permission to access the resource
    ForbiddenRequest = 4,
    /// Unavailable data, such as when the requested data is not available or cannot be retrieved
    UnavailableData = 5,
    /// Request timeout, such as when the server did not receive a response in time
    RequestTimeout = 6,
    /// Too many requests, such as when the server is overloaded or too many requests are made in a short period
    TooManyRequests = 7,
    /// Bad gateway, such as when the server encountered an error while communicating with an upstream server
    BadGateway = 8,
    /// Service unavailable, such as when the server is currently unavailable or undergoing maintenance
    ServiceUnavailable = 9,
    /// Gateway timeout, such as when the server did not receive a timely response from an upstream server
    GatewayTimeout = 10,
}

#[derive(Clone, PartialEq)]
pub struct KuraError {
    pub number: KuraErrorCode,
    pub reason: KuraErrorReason,
    pub description: Option<String>,
}

pub type KuraResult<T> = std::result::Result<T, KuraError>;

impl Display for KuraError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let description = match self.description.clone() {
            Some(description) => description,
            None => "-".to_string(),
        };
        write!(
            f,
            "ErrorCode: {}, ErrorReason: {}, Detaild: {}",
            self.number, self.reason, description
        )
    }
}

pub trait MapExternalToKuraResult<T, E> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T>;
}

impl<T, E: std::fmt::Debug> MapExternalToKuraResult<T, E> for std::result::Result<T, E> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T> {
        self.map_err(|error| KuraError {
            number: err_code,
            reason: err_reason,
            description: Some(format!("{description}: {error:?}")),
        })
    }
}

pub trait MapOptionToKuraError<T> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T>;
}

impl<T> MapOptionToKuraError<T> for Option<T> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T> {
        match self {
            Some(value) => Ok(value),
            None => Err(KuraError {
                number: err_code,
                reason: err_reason,
                description: Some(description),
            }),
        }
    }
}
