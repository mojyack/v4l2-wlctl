#pragma once
#include <linux/input.h>

#include "gawl/textrender.hpp"
#include "gawl/window-no-touch-callbacks.hpp"

#define CUTIL_NS vcw
#include "util/variant.hpp"
#undef CUTIL_NS

namespace vcw {
enum class ControlType {
    Int,
    Bool,
    Menu,
};

struct ValueRange {
    int min;
    int max;
    int step;
};

struct Control {
    virtual auto is_active() -> bool             = 0;
    virtual auto get_type() -> ControlType       = 0;
    virtual auto get_label() -> std::string_view = 0;
    virtual auto get_current() -> int            = 0;
    // for Int
    virtual auto get_range() -> ValueRange = 0;
    // for Menu
    virtual auto get_menu_size() -> size_t                        = 0;
    virtual auto get_menu_label(size_t index) -> std::string_view = 0;
    virtual auto get_menu_value(size_t index) -> int              = 0;

    virtual ~Control() {};
};

using ControlPtr = std::unique_ptr<Control>;

struct Label {
    std::string text;
};

struct QuitButton {
};

using Row = Variant<ControlPtr, Label, QuitButton>;

struct UserCallbacks {
    virtual auto set_control_value(Control& control, int value) -> void = 0;

    // return true to quit application
    virtual auto quit() -> bool {
        return true;
    }

    virtual ~UserCallbacks() {}
};

class Callbacks : public gawl::WindowNoTouchCallbacks {
  private:
    gawl::TextRender               font;
    std::vector<Row>&              rows;
    Control*                       focus_control = nullptr;
    gawl::Point                    pointer       = {-1, -1};
    std::shared_ptr<UserCallbacks> callbacks;

    auto calc_button_center(Control& ctrl) const -> double;
    auto draw_control(Control& ctrl, double y) -> void;
    auto draw_label(std::string_view text, bool is_quit, double y) -> void;
    auto draw_row(Row& row, double y) -> void;
    auto proc_control_click(Control& ctrl) -> void;
    auto quit() -> void;

  public:
    auto refresh() -> void override;
    auto close() -> void override;
    auto on_created(gawl::Window* window) -> coop::Async<bool> override;
    auto on_pointer(gawl::Point point) -> coop::Async<bool> override;
    auto on_click(uint32_t button, gawl::ButtonState state) -> coop::Async<bool> override;

    Callbacks(std::vector<Row>& rows, std::shared_ptr<UserCallbacks> callbacks);
};
} // namespace vcw
