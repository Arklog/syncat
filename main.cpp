#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <unordered_map>

#include "indicators/progress_bar.hpp"

const size_t buff_size = 32000;

int main(int argc, char **argv) {
    if (argc != 3)
        return 1;

    std::string infile = argv[1];
    std::string outfile = argv[2];

    std::ifstream ifs(infile);
    std::ofstream ofs(outfile);

    std::vector<char> buffer;
    buffer.reserve(buff_size);

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
        ifs.read(buffer.data(), buff_size);
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
