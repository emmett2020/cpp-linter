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
#include "tools/clang_format/general/impl.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <boost/regex.hpp>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include "tools/clang_format/general/reporter.h"
#include "tools/util.h"
#include "utils/shell.h"
#include "utils/util.h"

namespace linter::tool::clang_format {
  namespace {

    // Read a file and get this
    auto get_line_lens(std::string_view file_path) -> std::vector<uint32_t> {
      spdlog::trace("Enter clang_format::get_line_lens()");
      constexpr auto line_feed_len = 1;

      auto lines = std::vector<uint32_t>{};
      auto file  = std::ifstream{file_path.data()};
      auto temp  = std::string{};
      while (std::getline(file, temp)) {
        lines.emplace_back(temp.size() + line_feed_len);
      }
      return lines;
    }

    // offset starts from 0 while row/col starts from 1
    auto get_position(const std::vector<uint32_t> &lens, int offset)
      -> std::tuple<int32_t, int32_t> {
      spdlog::trace("Enter clang_format::get_position()");

      auto cur = uint32_t{0};
      for (int row = 0; row < lens.size(); ++row) {
        auto len = lens[row];
        if (offset >= cur && offset < cur + len) {
          return {row + 1, offset - cur + 1};
        }
        cur += len;
      }
      return {-1, -1};
    }

    inline auto xml_error(tinyxml2::XMLError err) -> std::string_view {
      spdlog::trace("Enter clang_format::xml_error() with err:{}", static_cast<int>(err));
      return tinyxml2::XMLDocument::ErrorIDToName(err);
    }

    inline auto xml_has_error(tinyxml2::XMLError err) -> bool {
      spdlog::trace("Enter clang_format::xml_has_error() with err:{}", static_cast<int>(err));
      return err != tinyxml2::XMLError::XML_SUCCESS;
    }

    auto parse_replacements_xml(std::string_view data) -> replacements_t {
      spdlog::trace("Enter clang_format::parse_replacements_xml()");

      // Names in replacements xml file.
      static constexpr auto offset_str       = "offset";
      static constexpr auto length_str       = "length";
      static constexpr auto replacements_str = "replacements";
      static constexpr auto replacement_str  = "replacement";

      // Start to parse given data to xml tree.
      auto doc = tinyxml2::XMLDocument{};
      auto err = doc.Parse(data.data());
      throw_if(xml_has_error(err),
               std::format("Parse replacements xml failed since: {}", xml_error(err)));
      throw_if(doc.NoChildren(),
               "Parse replacements xml failed since no children in replacements xml");

      // Find <replacements><replacement offset="xxx"
      // length="xxx">text</replacement></replacements>
      auto *replacements_ele = doc.FirstChildElement(replacements_str);
      throw_if(replacements_ele == nullptr,
               "Parse replacements xml failed since no child names 'replacements'");
      auto replacements = replacements_t{};

      // Empty replacement node is allowd here.
      auto *replacement_ele = replacements_ele->FirstChildElement(replacement_str);
      while (replacement_ele != nullptr) {
        auto replacement = replacement_t{};
        replacement_ele->QueryIntAttribute(offset_str, &replacement.offset);
        replacement_ele->QueryIntAttribute(length_str, &replacement.length);
        const auto *text = replacement_ele->GetText();
        if (text != nullptr) {
          replacement.data = text;
        }

        replacements.emplace_back(std::move(replacement));

        replacement_ele = replacement_ele->NextSiblingElement(replacement_str);
      }
      return replacements;
    }

    enum class output_style_t : std::uint8_t {
      formatted_source_code,
      replacement_xml
    };

    auto make_replacements_options(std::string_view file) -> std::vector<std::string> {
      spdlog::trace("Enter clang_format::make_replacements_options()");
      auto tool_opt = std::vector<std::string>{};
      tool_opt.emplace_back("--output-replacements-xml");
      tool_opt.emplace_back(file);
      return tool_opt;
    }

    auto make_source_code_options(std::string_view file) -> std::vector<std::string> {
      spdlog::trace("Enter clang_format::make_source_code_options()");
      auto tool_opt = std::vector<std::string>{};
      tool_opt.emplace_back(file);
      return tool_opt;
    }

    auto execute(const option_t &opt,
                 output_style_t output_style,
                 std::string_view repo,
                 std::string_view file) -> shell::result {
      spdlog::trace("Enter clang_format::execute()");

      auto tool_opt     = output_style == output_style_t::formatted_source_code
                          ? make_source_code_options(file)
                          : make_replacements_options(file);
      auto tool_opt_str = tool_opt | std::views::join_with(' ') | std::ranges::to<std::string>();
      spdlog::info("Running command: {} {}", opt.binary, tool_opt_str);

      return shell::execute(opt.binary, tool_opt, repo);
    }

  } // namespace

  auto clang_format_general::check_single_file(
    [[maybe_unused]] const runtime_context &ctx,
    const std::string &root_dir,
    const std::string &file) const -> per_file_result {
    spdlog::trace("Enter base_clang_format::apply_on_single_file()");

    auto xml_res       = execute(option, output_style_t::replacement_xml, root_dir, file);
    auto result        = per_file_result{};
    result.file_path   = file;
    result.tool_stdout = xml_res.std_out;
    result.tool_stderr = xml_res.std_err;
    if (xml_res.exit_code != 0) {
      result.passed = false;
      return result;
    }

    auto replacements   = parse_replacements_xml(xml_res.std_out);
    result.replacements = std::move(replacements);

    if (option.needs_formatted_source_code) {
      spdlog::debug("Execute clang-format again to get formatted source code.");
      auto code_res       = execute(option, output_style_t::formatted_source_code, root_dir, file);
      result.tool_stdout += "\n" + code_res.std_out;
      result.tool_stderr += "\n" + code_res.std_err;
      if (code_res.exit_code != 0) {
        result.passed = false;
        return result;
      }
      result.formatted_source_code = code_res.std_out;
    }

    return result;
  }

  void clang_format_general::check(const runtime_context &context) {
    const auto root_dir = context.repo_path;
    const auto files    = context.changed_files;
    for (const auto &file: files) {
      if (filter_file(option.source_filter_iregex, file)) {
        result.ignored.push_back(file);
        spdlog::debug("file is ignored {} by {}", file, option.binary);
        continue;
      }

      auto per_file_result = check_single_file(context, root_dir, file);
      if (per_file_result.passed) {
        spdlog::info("file: {} passes {} check.", file, option.binary);
        result.passes[file] = std::move(per_file_result);
        continue;
      }

      spdlog::error("file: {} doesn't pass {} check.", file, option.binary);
      result.fails[file] = std::move(per_file_result);

      if (option.enabled_fastly_exit) {
        spdlog::info("{} fastly exit since check failed", option.binary);
        result.final_passed  = false;
        result.fastly_exited = true;
        return;
      }
    }

    result.final_passed = true;
  }

  auto clang_format_general::get_reporter() -> reporter_base_ptr {
    return std::make_unique<reporter_t>(option, result);
  }

} // namespace linter::tool::clang_format
