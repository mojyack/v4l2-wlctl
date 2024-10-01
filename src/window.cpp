#include "window.hpp"
#include "gawl/application.hpp"
#include "gawl/fc.hpp"
#include "gawl/misc.hpp"
#include "gawl/window.hpp"
#include "macros/unwrap.hpp"

namespace vcw {
namespace {
constexpr auto row_height           = 30.0;
constexpr auto row_separetor_height = 2.0;
constexpr auto color_back           = gawl::Color{1. * 0x23 / 0xFF, 1. * 0x2C / 0xFF, 1. * 0x31 / 0xFF, 1};
constexpr auto color_front          = gawl::Color{1. * 0xC5 / 0xFF, 1. * 0xC8 / 0xFF, 1. * 0xC6 / 0xFF, 1};
constexpr auto color_front_focus    = gawl::Color{1. * 0xA5 / 0xFF, 1. * 0xA8 / 0xFF, 1. * 0xA6 / 0xFF, 1};
constexpr auto color_front_inactive = gawl::Color{1. * 0x69 / 0xFF, 1. * 0x6A / 0xFF, 1. * 0x6B / 0xFF, 1};
constexpr auto slider_button_width  = 60.0;
// constexpr auto slider_button_height = row_height * 0.9;
} // namespace

auto Callbacks::calc_button_center(Control& ctrl) const -> double {
    const auto [width, height]  = window->get_window_size();
    const auto [min, max, step] = ctrl.get_range();
    const auto current          = ctrl.get_current();
    const auto button_center    = slider_button_width / 2 + (width - slider_button_width) * (current - min) / (max - min);
    return button_center;
}

auto Callbacks::draw_control(Control& ctrl, const double y) -> void {
    const auto [width, height] = window->get_window_size();
    const auto div_rect        = gawl::Rectangle{{0, y}, {1. * width, y + row_height - row_separetor_height}};
    const auto label_color     = ctrl.is_active() ? gawl::Color{1, 1, 1, 1} : color_front_inactive;
    const auto current         = ctrl.get_current();

    gawl::draw_rect(*window, div_rect, color_back);
    gawl::draw_rect(*window, {{0, y + row_height - row_separetor_height}, {1. * width, y + row_height}}, color_front);
    switch(ctrl.get_type()) {
    case ControlType::Int: {
        const auto [min, max, step] = ctrl.get_range();
        font.draw_fit_rect(*window, div_rect, label_color, ctrl.get_label(), int(div_rect.height() * 0.6), gawl::Align::Center);
        font.draw_fit_rect(*window, div_rect, label_color, std::to_string(min), int(div_rect.height() * 0.5), gawl::Align::Left);
        font.draw_fit_rect(*window, div_rect, label_color, std::to_string(max), int(div_rect.height() * 0.5), gawl::Align::Right);

        const auto button_center = calc_button_center(ctrl);
        const auto button_rect   = gawl::Rectangle{
              {button_center - slider_button_width / 2, div_rect.a.y},
              {button_center + slider_button_width / 2, div_rect.a.y + div_rect.height()}};
        gawl::draw_rect(*window, button_rect, &ctrl == focus_control ? color_front_focus : color_front);
        font.draw_fit_rect(*window, button_rect, label_color, std::to_string(current), int(div_rect.height() * 0.6), gawl::Align::Center);
    } break;
    case ControlType::Bool: {
        if(current) {
            gawl::draw_rect(*window, {div_rect.a, {div_rect.b.x / 2, div_rect.b.y}}, color_front);
            font.draw_fit_rect(*window, div_rect, label_color, "On", int(div_rect.height() * 0.5), gawl::Align::Left);
        } else {
            gawl::draw_rect(*window, {{div_rect.b.x / 2, div_rect.a.y}, div_rect.b}, color_front);
            font.draw_fit_rect(*window, div_rect, label_color, "Off", int(div_rect.height() * 0.5), gawl::Align::Right);
        }
        font.draw_fit_rect(*window, div_rect, label_color, ctrl.get_label(), int(div_rect.height() * 0.6), gawl::Align::Center);
    } break;
    case ControlType::Menu: {
        const auto menu_width = div_rect.width() / (ctrl.get_menu_size());
        for(auto i = 0u; i < ctrl.get_menu_size(); i += 1) {
            const auto x         = i * menu_width;
            const auto menu_rect = gawl::Rectangle{{x, y}, {x + menu_width, y + div_rect.height()}};
            if(ctrl.get_current() == ctrl.get_menu_value(i)) {
                gawl::draw_rect(*window, menu_rect, color_front);
            }
            font.draw_fit_rect(*window, menu_rect, label_color, ctrl.get_menu_label(i), int(div_rect.height() * 0.5), gawl::Align::Center);
            gawl::draw_rect(*window, {{x, y}, {x + 2, y + div_rect.height()}}, color_front);
        }
    } break;
    }
}

auto Callbacks::draw_label(const std::string_view text, const bool is_quit, const double y) -> void {
    const auto [width, height] = window->get_window_size();
    const auto div_rect        = gawl::Rectangle{{0, y}, {1. * width, y + row_height}};

    gawl::draw_rect(*window, div_rect, color_back);
    const auto color = is_quit ? gawl::Color{1, 0, 0, 1} : gawl::Color{1, 1, 1, 1};
    font.draw_fit_rect(*window, div_rect, color, text, int(row_height * 0.6), gawl::Align::Center);
}

auto Callbacks::draw_row(Row& row, const double y) -> void {
    switch(row.get_index()) {
    case Row::index_of<ControlPtr>:
        draw_control(*row.as<ControlPtr>(), y);
        break;
    case Row::index_of<Label>:
        draw_label(row.as<Label>().text, false, y);
        break;
    case Row::index_of<QuitButton>:
        draw_label("Quit", false, y);
        break;
    }
}

auto Callbacks::proc_control_click(Control& ctrl) -> void {
    if(!ctrl.is_active()) {
        return;
    }
    switch(ctrl.get_type()) {
    case ControlType::Int: {
        const auto button_center = calc_button_center(ctrl);
        if(pointer.x >= button_center - slider_button_width && pointer.x < button_center + slider_button_width) {
            focus_control = &ctrl;
            window->refresh();
        }
    } break;
    case ControlType::Bool: {
        callbacks->set_control_value(ctrl, !ctrl.get_current());
        window->refresh();
    } break;
    case ControlType::Menu: {
        const auto [width, height] = window->get_window_size();
        const auto menu_width      = 1. * width / ctrl.get_menu_size();
        const auto num             = pointer.x / menu_width;
        callbacks->set_control_value(ctrl, ctrl.get_menu_value(num));
        window->refresh();
    } break;
    }
}

auto Callbacks::quit() -> void {
    if(callbacks->quit()) {
        application->quit();
    }
}

auto Callbacks::refresh() -> void {
    for(auto i = 0u; i < rows.size(); i += 1) {
        const auto y = i * row_height;
        draw_row(rows[i], y);
    }
}

auto Callbacks::close() -> void {
    quit();
}

auto Callbacks::on_pointer(const gawl::Point& point) -> void {
    pointer = point;
    if(focus_control == nullptr) {
        return;
    }
    auto& ctrl = *focus_control;
    switch(focus_control->get_type()) {
    case ControlType::Int: {
        const auto [width, height]  = window->get_window_size();
        const auto [min, max, step] = ctrl.get_range();
        const auto range            = (max - min + 1) / step;
        const auto value            = min + std::clamp(int32_t((point.x - slider_button_width / 2) / (width - slider_button_width) * range), 0, range - 1) * step;
        callbacks->set_control_value(ctrl, value);
        window->refresh();
    } break;
    default:
        break;
    }
}

auto Callbacks::on_click(const uint32_t button, const gawl::ButtonState state) -> void {
    if(button != BTN_LEFT) {
        return;
    }
    if(pointer.y >= rows.size() * row_height || state != gawl::ButtonState::Press) {
        if(focus_control != nullptr) {
            focus_control = nullptr;
            window->refresh();
        } else {
            focus_control = nullptr;
        }
        return;
    }
    auto& row = rows[(pointer.y / row_height)];
    switch(row.get_index()) {
    case Row::index_of<ControlPtr>:
        proc_control_click(*row.as<ControlPtr>());
        break;
    case Row::index_of<Label>:
        break;
    case Row::index_of<QuitButton>:
        quit();
        break;
    }
}

auto Callbacks::init() -> bool {
    unwrap(fontpath, gawl::find_fontpath_from_name("Noto Sans CJK JP"));
    font = gawl::TextRender({fontpath}, 32);
    return true;
}

Callbacks::Callbacks(std::vector<Row>& rows, std::shared_ptr<UserCallbacks> callbacks)
    : rows(rows),
      callbacks(callbacks) {
}
} // namespace vcw
