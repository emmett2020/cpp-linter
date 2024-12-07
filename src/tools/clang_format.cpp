#include "clang_format.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>
#include <spdlog/spdlog.h>
namespace pt = boost::property_tree;

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

    auto parse_replacements_xml(std::string_view data) -> replacements {
      std::istringstream iss(data.data());
      auto tree = pt::ptree{};
      pt::read_xml(iss, tree);

      auto replaces = replacements{};
      for (auto node: tree.get_child("xml.replacements")) {
        auto replace   = replacement{};
        replace.offset = node.second.get<std::size_t>("<xmlattr>.offset");
        replace.length = node.second.get<std::size_t>("<xmlattr>.length");
        replace.data   = node.second.data();
        replaces.emplace_back(std::move(replace));
      }
      return replaces;
    }

    auto execute(const option &option, std::string_view repo, std::string_view file)
      -> shell::result {
      auto opts = std::vector<std::string>{};
      opts.emplace_back("output-replacements-xml");
      opts.emplace_back(file);

      auto arg_str = opts | std::views::join_with(' ') | std::ranges::to<std::string>();
      spdlog::info("Running command: {} {}", option.clang_format_binary, arg_str);

      return shell::execute(option.clang_format_binary, opts, repo);
    }

  } // namespace

  auto run(const option &option, const std::string &repo, const std::string &file) -> result {
    spdlog::info("Start to run clang-format");
    auto [ec, std_out, std_err] = execute(option, repo, file);
    spdlog::trace("clang-format original output:\nreturn code: {}\nstdout:\n{}stderr:\n{}",
                  ec,
                  std_out,
                  std_err);
    spdlog::info("Successfully ran clang-format, now start to parse the output of it.");

    auto replacements = parse_replacements_xml(std_out);

    auto res          = result{};
    res.pass          = replacements.empty();
    res.file          = file;
    res.origin_stderr = std_err;
    res.replaces      = std::move(replacements);
    return {};
  }

} // namespace linter::clang_format
