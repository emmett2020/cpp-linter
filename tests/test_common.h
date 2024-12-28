/*
 * Copyright (c) 2024 Emmett Zhang
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cctype>
#include <filesystem>

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <git2/diff.h>
#include <spdlog/spdlog.h>

#include "utils/git_utils.h"

// Get temporary repository directory.
auto get_temp_repo_dir() -> std::filesystem::path;

// Remove repository.
void remove_repo();

// Create temporary repository. If repository already exists, it'll be
// refreshed.
void create_temp_repo();

// Create file in temporary repository.
void create_temp_file(const std::string &file_name, const std::string &content);

// Create files in temporary repository. Each file contains the same content.
void create_temp_files(const std::vector<std::string> &file_names,
                       const std::string &content);

// Append the given content into the given file.
void append_content_to_file(const std::string &file,
                            const std::string &content);

// Initialize a basic repo for futhure test.
auto init_basic_repo() -> linter::git::repo_ptr;

// Initialize a basic repo for futhure test.
auto init_repo_with_commit(const std::vector<std::string> &files,
                           const std::string &commit_message = "")
    -> std::tuple<linter::git::repo_ptr, linter::git::commit_ptr>;
