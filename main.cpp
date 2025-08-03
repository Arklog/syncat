#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <unordered_map>
#include "indicators/progress_bar.hpp"
#include "argparse/argparse.hpp"

constexpr size_t buff_size = 32 * 1 << 20; //32mb

void parse_size(size_t &size, std::string arg) {
    static const std::unordered_map<std::string, float> size_map = {
        {"kb", 1 << 10},
        {"mb", 1 << 20},
        {"gb", 1 << 30},
        {"tb", 1 << 40}
    };
    std::ranges::for_each(arg, ::tolower);

    auto iter = arg.begin();
    while (!::isalpha(*iter))
        ++iter;

    std::string value(arg.begin(), iter);
    std::string size_type(iter, arg.end());

    auto fsize = ::strtof(value.c_str(), nullptr);
    size = static_cast<size_t>(fsize);

    if (size_type.empty())
        return;
    size = static_cast<size_t>(size_map.at(size_type) * fsize);
}

struct {
    std::string infile{};
    std::string outfile{};
    size_t buff_size = buff_size;
} options;

int main(int argc, char **argv) {
    auto program = argparse::ArgumentParser{"syncat"};

    program.add_argument("infile").required().help("input file").store_into(options.infile);
    program.add_argument("outfile").required().help("output file").store_into(options.outfile);
    program.add_argument("--bs").help("buffer size").action([] (std::string arg) {
        parse_size(options.buff_size, std::move(arg));
    });

    program.parse_args(argc, argv);

    std::ifstream ifs(options.infile);
    std::ofstream ofs(options.outfile);

    std::vector<char> buffer;
    buffer.reserve(options.buff_size);

    auto fsize = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    fsize = ifs.tellg() - fsize;
    ifs.seekg(0, std::ios::beg);

    auto progress_bar = indicators::ProgressBar{
        indicators::option::MaxProgress{fsize},
        indicators::option::ShowElapsedTime{true},
        indicators::option::ShowPercentage{true},
    };

    std::unordered_map<size_t, bool> sync_map{};
    auto progress = ifs.tellg();

    while (!ifs.eof()) {
        ifs.read(buffer.data(), options.buff_size);
        ofs.write(buffer.data(), ifs.gcount());

        auto double_progress = static_cast<double>(progress) / static_cast<double>(fsize);
        auto progress_idx = static_cast<size_t>(double_progress * 100.0);
        if (!sync_map.contains(progress_idx)) {
            sync_map.emplace(progress_idx, true);
            ::fsync(ofs.native_handle());
        }

        progress += ifs.gcount();
        progress_bar.set_progress(progress);
        progress_bar.print_progress();
    }
}
