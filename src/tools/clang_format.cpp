#include "clang_format.h"

#include <cctype>
#include <format>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <boost/regex.hpp>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include "utils/shell.h"
#include "utils/util.h"

namespace linter::clang_format {
  using namespace std::string_view_literals;

  namespace {
    auto get_line_lens(std::string_view file_path) -> std::vector<uint32_t> {
      auto lines = std::vector<uint32_t>{};
      auto file  = std::ifstream{file_path.data()};

      auto temp = std::string{};
      while (std::getline(file, temp)) {
        // LF
        lines.emplace_back(temp.size() + 1);
      }
      return lines;
    }

    // offset starts from 0 while row/col starts from 1
    auto get_position(const std::vector<uint32_t> &lens, int offset)
      -> std::tuple<int32_t, int32_t> {
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
      return tinyxml2::XMLDocument::ErrorIDToName(err);
    }

    inline auto xml_has_error(tinyxml2::XMLError err) -> bool {
      return err != tinyxml2::XMLError::XML_SUCCESS;
    }

    auto parse_replacements_xml(std::string_view data) -> replacements_type {
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

      // Find <replacements><replacement offset="xxx" length="xxx">text</replacement></replacements>
      auto *replacements_ele = doc.FirstChildElement(replacements_str);
      throw_if(replacements_ele == nullptr,
               "Parse replacements xml failed since no child names 'replacements'");
      auto replacements = replacements_type{};

      // Empty replacement node is allowd here.
      auto *replacement_ele = replacements_ele->FirstChildElement(replacement_str);
      while (replacement_ele != nullptr) {
        auto replacement = replacement_type{};
        replacement_ele->QueryIntAttribute(offset_str, &replacement.offset);
        replacement_ele->QueryIntAttribute(length_str, &replacement.length);
        const auto *text = replacements_ele->GetText();
        if (text != nullptr) {
          replacement.data = text;
        }

        print_replacement(replacement);
        replacements.emplace_back(std::move(replacement));

        replacement_ele = replacements_ele->NextSiblingElement(replacement_str);
      }
      return replacements;
    }

    auto execute(const option &option, std::string_view repo, std::string_view file)
      -> shell::result {
      auto opts = std::vector<std::string>{};
      opts.emplace_back("--output-replacements-xml");
      opts.emplace_back(file);

      auto arg_str = opts | std::views::join_with(' ') | std::ranges::to<std::string>();
      spdlog::info("Running command: {} {}", option.clang_format_binary, arg_str);

      return shell::execute(option.clang_format_binary, opts, repo);
    }

  } // namespace

  void print_replacement(const replacement_type &replacement) {
    spdlog::trace("offset: {}, length: {}, data: {}",
                  replacement.offset,
                  replacement.length,
                  replacement.data);
  }

  auto run(const option &option, const std::string &repo, const std::string &file) -> result {
    spdlog::info("Start to run clang-format");
    auto [ec, std_out, std_err] = execute(option, repo, file);
    spdlog::trace("clang-format original output:\nreturn code: {}\nstdout:\n{}stderr:\n{}",
                  ec,
                  std_out,
                  std_err);
    spdlog::info("Successfully ran clang-format, now start to parse the output of it.");

    if (ec != 0) {
      return {.pass = false, .file = file, .replacements = {}, .origin_stderr = std_err};
    }

    auto replacements = parse_replacements_xml(std_out);

    auto res          = result{};
    res.pass          = replacements.empty();
    res.file          = file;
    res.origin_stderr = std_err;
    res.replacements  = std::move(replacements);
    return res;
  }

} // namespace linter::clang_format
