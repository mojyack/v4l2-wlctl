#include <fcntl.h>

#include "assert.hpp"
#include "v4l2.hpp"

auto main(const int argc, const char* argv[]) -> int {
    if(argc < 4) {
        return 1;
    }

    auto fd = open(argv[1], O_RDWR);
    DYN_ASSERT(fd != -1);

    for(auto& ctrl : v4l2::query_controls(fd)) {
        for(auto key = 2; key + 1 < argc; key += 2) {
            if(std::string_view(ctrl.name) == argv[key]) {
                v4l2::set_control(fd, ctrl.id, std::stoi(argv[key + 1]));
                printf("\"%s\" = %s\n", argv[key], argv[key + 1]);
            }
        }
    }

    return 0;
}
