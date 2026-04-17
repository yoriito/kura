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

use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
pub struct Args {
    #[arg(
        short,
        long,
        long_help = "relative path to config yml file. this is a path relative to one of the following absolute paths: \n\
                     * home directory \n\
                     * /etc/yoriito/viss \n\
                     * /usr/local/etc/yoriito/viss \n\
                     * current directory "
    )]
    pub config_file_path: String,
    #[arg(short, long, default_value_t = false)]
    pub verbose: bool,
    #[arg(short, long, default_value_t = false)]
    pub fake_platform: bool,
}
