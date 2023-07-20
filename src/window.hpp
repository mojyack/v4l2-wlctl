#pragma once
#define GAWL_KEYCODE
#define GAWL_MOUSE
#include <gawl/fc.hpp>
#include <gawl/polygon.hpp>
#include <gawl/textrender.hpp>
#include <gawl/wayland/gawl.hpp>

#define CUTIL_NS vcw
#include "util/variant.hpp"
#undef CUTIL_NS

namespace vcw {
enum class ControlType {
    Int,
    Bool,
    Menu,
};

template <class T>
concept Control = requires(T& ctrl) {
    { ctrl.inactive() } -> std::same_as<bool>;
    { ctrl.get_type() } -> std::same_as<ControlType>;
    { ctrl.get_label() } -> std::same_as<std::string_view>;
    { ctrl.get_max() } -> std::same_as<int>;
    { ctrl.get_min() } -> std::same_as<int>;
    { ctrl.get_step() } -> std::same_as<int>;
    { ctrl.get_current() } -> std::same_as<int>;
    { ctrl.get_menu_size() } -> std::same_as<size_t>;
    { ctrl.get_menu_label(0) } -> std::same_as<std::string_view>;
    { ctrl.get_menu_value(0) } -> std::same_as<int>;
};

template <Control T>
using ApplyControl = bool(T&, int value); // returns are controls invalidated

enum class LabelType {
    Normal,
    Quit,
};

template <LabelType>
struct Label {
    int         id;
    std::string text;
};

template <Control T>
class Window {
  public:
    using Row = Variant<T, Label<LabelType::Normal>, Label<LabelType::Quit>>;

  private:
    gawl::Window<Window>&          window;
    gawl::TextRender               font;
    std::vector<Row>&              rows;
    std::function<ApplyControl<T>> apply_control;
    std::function<void()>          quit;
    T*                             focus_control = nullptr;
    gawl::Point                    pointer       = {-1, -1};

    static constexpr auto row_height           = 30.0;
    static constexpr auto row_separetor_height = 2.0;
    static constexpr auto color_back           = gawl::Color{1. * 0x23 / 0xFF, 1. * 0x2C / 0xFF, 1. * 0x31 / 0xFF, 1};
    static constexpr auto color_front          = gawl::Color{1. * 0xC5 / 0xFF, 1. * 0xC8 / 0xFF, 1. * 0xC6 / 0xFF, 1};
    static constexpr auto color_front_focus    = gawl::Color{1. * 0xA5 / 0xFF, 1. * 0xA8 / 0xFF, 1. * 0xA6 / 0xFF, 1};
    static constexpr auto color_front_inactive = gawl::Color{1. * 0x69 / 0xFF, 1. * 0x6A / 0xFF, 1. * 0x6B / 0xFF, 1};
    static constexpr auto slider_button_width  = 60.0;
    static constexpr auto slider_button_height = row_height * 0.9;

    auto calc_button_center(const T& ctrl) const -> double {
        const auto [ww, wh]      = window.get_window_size();
        const auto button_center = slider_button_width / 2 + (ww - slider_button_width) * (ctrl.get_current() - ctrl.get_min()) / (ctrl.get_max() - ctrl.get_min());
        return button_center;
    }

    auto draw_control(const T& ctrl, const double y) -> void {
        const auto [ww, wh]    = window.get_window_size();
        const auto div_rect    = gawl::Rectangle{{0, y}, {1. * ww, y + row_height - row_separetor_height}};
        const auto label_color = !ctrl.inactive() ? gawl::Color{1, 1, 1, 1} : color_front_inactive;

        gawl::draw_rect(window, div_rect, color_back);
        gawl::draw_rect(window, {{0, y + row_height - row_separetor_height}, {1. * ww, y + row_height}}, color_front);
        switch(ctrl.get_type()) {
        case ControlType::Int: {
            font.draw_fit_rect(window, div_rect, label_color, ctrl.get_label(), int(div_rect.height() * 0.6), gawl::Align::Center);
            font.draw_fit_rect(window, div_rect, label_color, std::to_string(ctrl.get_min()), int(div_rect.height() * 0.5), gawl::Align::Left);
            font.draw_fit_rect(window, div_rect, label_color, std::to_string(ctrl.get_max()), int(div_rect.height() * 0.5), gawl::Align::Right);

            const auto button_center = calc_button_center(ctrl);
            const auto button_rect   = gawl::Rectangle{
                  {button_center - slider_button_width / 2, div_rect.a.y},
                  {button_center + slider_button_width / 2, div_rect.a.y + div_rect.height()}};
            gawl::draw_rect(window, button_rect, &ctrl == focus_control ? color_front_focus : color_front);
            font.draw_fit_rect(window, button_rect, label_color, std::to_string(ctrl.get_current()), int(div_rect.height() * 0.6), gawl::Align::Center);
        } break;
        case ControlType::Bool: {
            if(ctrl.get_current()) {
                gawl::draw_rect(window, {div_rect.a, {div_rect.b.x / 2, div_rect.b.y}}, color_front);
                font.draw_fit_rect(window, div_rect, label_color, "On", int(div_rect.height() * 0.5), gawl::Align::Left);
            } else {
                gawl::draw_rect(window, {{div_rect.b.x / 2, div_rect.a.y}, div_rect.b}, color_front);
                font.draw_fit_rect(window, div_rect, label_color, "Off", int(div_rect.height() * 0.5), gawl::Align::Right);
            }
            font.draw_fit_rect(window, div_rect, label_color, ctrl.get_label(), int(div_rect.height() * 0.6), gawl::Align::Center);
        } break;
        case ControlType::Menu: {
            const auto menu_width = div_rect.width() / (ctrl.get_menu_size());
            for(auto i = 0u; i < ctrl.get_menu_size(); i += 1) {
                const auto x         = i * menu_width;
                const auto menu_rect = gawl::Rectangle{{x, y}, {x + menu_width, y + div_rect.height()}};
                if(ctrl.get_current() == ctrl.get_menu_value(i)) {
                    gawl::draw_rect(window, menu_rect, color_front);
                }
                font.draw_fit_rect(window, menu_rect, label_color, ctrl.get_menu_label(i), int(div_rect.height() * 0.5), gawl::Align::Center);
                gawl::draw_rect(window, {{x, y}, {x + 2, y + div_rect.height()}}, color_front);
            }
        } break;
        }
    }

    template <LabelType type>
    auto draw_label(const Label<type>& label, const double y) -> void {
        const auto [ww, wh] = window.get_window_size();
        const auto div_rect = gawl::Rectangle{{0, y}, {1. * ww, y + row_height}};

        gawl::draw_rect(window, div_rect, color_back);
        switch(type) {
        case LabelType::Normal:
            font.draw_fit_rect(window, div_rect, {1, 1, 1, 1}, label.text, int(row_height * 0.6), gawl::Align::Center);
            break;
        case LabelType::Quit:
            font.draw_fit_rect(window, div_rect, {1, 0, 0, 1}, label.text, int(row_height * 0.6), gawl::Align::Center);
            break;
        }
    }

    auto draw_row(const Row& row, const double y) -> void {
        switch(row.get_index()) {
        case Row::template index_of<T>:
            draw_control(row.template as<T>(), y);
            break;
        case Row::template index_of<Label<LabelType::Normal>>:
            draw_label(row.template as<Label<LabelType::Normal>>(), y);
            break;
        case Row::template index_of<Label<LabelType::Quit>>:
            draw_label(row.template as<Label<LabelType::Quit>>(), y);
            break;
        }
    }

    auto proc_control_click(T& ctrl) -> void {
        if(ctrl.inactive()) {
            return;
        }
        switch(ctrl.get_type()) {
        case ControlType::Int: {
            const auto button_center = calc_button_center(ctrl);
            if(pointer.x >= button_center - slider_button_width && pointer.x < button_center + slider_button_width) {
                focus_control = &ctrl;
                window.refresh();
            }
        } break;
        case ControlType::Bool: {
            apply_control(ctrl, !ctrl.get_current());
            window.refresh();
        } break;
        case ControlType::Menu: {
            const auto [ww, wh]   = window.get_window_size();
            const auto menu_width = 1. * ww / ctrl.get_menu_size();
            const auto num        = pointer.x / menu_width;
            apply_control(ctrl, ctrl.get_menu_value(num));
            window.refresh();
        } break;
        }
    }

  public:
    auto refresh_callback() -> void {
        for(auto i = 0u; i < rows.size(); i += 1) {
            const auto y = i * row_height;
            draw_row(rows[i], y);
        }
    }

    auto pointer_move_callback(const gawl::Point& point) -> void {
        pointer = point;
        if(focus_control == nullptr) {
            return;
        }
        auto& ctrl = *focus_control;
        switch(focus_control->get_type()) {
        case ControlType::Int: {
            const auto [ww, wh] = window.get_window_size();
            const auto range    = (ctrl.get_max() - ctrl.get_min() + 1) / ctrl.get_step();
            const auto value    = ctrl.get_min() + std::clamp(int32_t((point.x - slider_button_width / 2) / (ww - slider_button_width) * range), 0, range - 1) * ctrl.get_step();
            if(apply_control(ctrl, value)) {
                focus_control = nullptr;
            }
            window.refresh();
        } break;
        default:
            break;
        }
    }

    auto click_callback(const uint32_t button, const gawl::ButtonState state) -> void {
        if(button != BTN_LEFT) {
            return;
        }
        if(pointer.y >= rows.size() * row_height || state != gawl::ButtonState::Press) {
            if(focus_control != nullptr) {
                focus_control = nullptr;
                window.refresh();
            } else {
                focus_control = nullptr;
            }
            return;
        }
        auto& row = rows[(pointer.y / row_height)];
        switch(row.get_index()) {
        case Row::template index_of<T>:
            proc_control_click(row.template as<T>());
            break;
        case Row::template index_of<Label<LabelType::Normal>>:
            break;
        case Row::template index_of<Label<LabelType::Quit>>:
            std::quick_exit(0);
            break;
        }
    }

    Window(gawl::Window<Window>& window, std::vector<Row>& rows, std::function<ApplyControl<T>> apply_control, std::function<void()> quit)
        : window(window),
          font({gawl::find_fontpath_from_name("Noto Sans CJK JP").unwrap().data()}, 32),
          rows(rows),
          apply_control(apply_control),
          quit(quit) {}
};
} // namespace vcw
