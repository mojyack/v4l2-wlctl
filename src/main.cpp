#include <fcntl.h>

#include "v4l2.hpp"
#include "window.hpp"

struct Control {
    v4l2::Control ctrl;

    auto inactive() const -> bool {
        return ctrl.ro || ctrl.inactive;
    }

    auto get_type() const -> vcw::ControlType {
        switch(ctrl.type) {
        case v4l2::ControlType::Int:
            return vcw::ControlType::Int;
        case v4l2::ControlType::Bool:
            return vcw::ControlType::Bool;
        case v4l2::ControlType::Menu:
            return vcw::ControlType::Menu;
        }
    }

    auto get_label() const -> std::string_view {
        return ctrl.name;
    }

    auto get_max() const -> int {
        return ctrl.max;
    }

    auto get_min() const -> int {
        return ctrl.min;
    }

    auto get_step() const -> int {
        return ctrl.step;
    }

    auto get_current() const -> int {
        return ctrl.current;
    }

    auto get_menu_size() const -> size_t {
        return ctrl.menus.size();
    }

    auto get_menu_label(const int i) const -> std::string_view {
        return ctrl.menus[i].name;
    }

    auto get_menu_value(const int i) const -> int {
        return int(ctrl.menus[i].index);
    }
};

using Window = vcw::Window<Control>;

auto controls_to_rows(std::vector<v4l2::Control> controls) -> std::vector<Window::Row> {
    auto ret = std::vector<Window::Row>();
    for(auto& ctrl : controls) {
        if(ctrl.type == v4l2::ControlType::Menu) {
            ret.emplace_back(vcw::Tag<vcw::Label<vcw::LabelType::Normal>>(), vcw::Label<vcw::LabelType::Normal>{0, ctrl.name});
        }
        ret.emplace_back(vcw::Tag<Control>(), Control{std::move(ctrl)});
    }
    ret.emplace_back(vcw::Tag<vcw::Label<vcw::LabelType::Quit>>(), vcw::Label<vcw::LabelType::Quit>{0, "Quit"});

    return ret;
}

auto main(const int argc, const char* argv[]) -> int {
    if(argc < 2) {
        return 1;
    }

    auto fd            = open(argv[1], O_RDWR);
    auto rows          = controls_to_rows(v4l2::query_controls(fd));
    auto apply_control = std::function([fd, &rows](Control& ctrl_, const int value) -> bool {
        auto& ctrl   = ctrl_.ctrl;
        ctrl.current = value;
        v4l2::set_control(fd, ctrl.id, ctrl.current);
        if(ctrl.type == v4l2::ControlType::Bool || ctrl.type == v4l2::ControlType::Menu) {
            rows = controls_to_rows(v4l2::query_controls(fd)); // reload for newly activated/inactivated controls;
            return true;
        }
        return false;
    });

    auto app = gawl::Application();
    app.open_window<Window>({.title = "v4l2-wlctl", .manual_refresh = true}, rows, apply_control);
    app.run();
    return 0;
}
