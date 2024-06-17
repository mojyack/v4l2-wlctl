#include <fcntl.h>

#include "macros/unwrap.hpp"
#include "util/assert.hpp"
#include "util/charconv.hpp"
#include "v4l2.hpp"

namespace {
auto run(const int argc, const char* argv[]) -> bool {
    assert_b(argc >= 4);

    const auto fd = open(argv[1], O_RDWR);
    assert_b(fd != -1);

    for(auto& ctrl : v4l2::query_controls(fd)) {
        for(auto key = 2; key + 1 < argc; key += 2) {
            if(std::string_view(ctrl.name) != argv[key]) {
                continue;
            }
            unwrap_ob(value, from_chars<int>(argv[key + 1]), "invalid argument");
            printf("\"%s\" = %s\n", argv[key], argv[key + 1]);
            if(!v4l2::set_control(fd, ctrl.id, value)) {
                WARN("failed to set control value");
            }
        }
    }
    return true;
}
} // namespace

auto main(const int argc, const char* argv[]) -> int {
    return run(argc, argv) ? 0 : 1;
}
