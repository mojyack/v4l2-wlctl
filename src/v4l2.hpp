#pragma once
#include <optional>
#include <vector>

namespace v4l2 {
enum class ControlType {
    Int,
    Bool,
    Menu,
};

struct ControlMenu {
    char     name[32];
    uint32_t index;
};

struct Control {
    uint32_t                 id;
    ControlType              type;
    char                     name[32];
    int32_t                  max;
    int32_t                  min;
    int32_t                  step;
    int32_t                  current;
    std::vector<ControlMenu> menus;

    // flags
    bool ro;
    bool inactive;
};

auto query_controls(int fd) -> std::vector<Control>;
auto get_control(int fd, uint32_t id) -> std::optional<int32_t>;
auto set_control(int fd, uint32_t id, int32_t value) -> bool;
} // namespace v4l2
