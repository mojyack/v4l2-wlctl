#include <fcntl.h>

#include "gawl/wayland/application.hpp"
#include "macros/assert.hpp"
#include "v4l2.hpp"
#include "window.hpp"

struct Control : vcw::Control {
    v4l2::Control ctrl;

    auto is_active() -> bool override {
        return !ctrl.ro && !ctrl.inactive;
    }

    auto get_type() -> vcw::ControlType override {
        switch(ctrl.type) {
        case v4l2::ControlType::Int:
            return vcw::ControlType::Int;
        case v4l2::ControlType::Bool:
            return vcw::ControlType::Bool;
        case v4l2::ControlType::Menu:
            return vcw::ControlType::Menu;
        }
    }

    auto get_label() -> std::string_view override {
        return ctrl.name;
    }

    auto get_range() -> vcw::ValueRange override {
        return {ctrl.min, ctrl.max, ctrl.step};
    }

    auto get_current() -> int override {
        return ctrl.current;
    }

    auto get_menu_size() -> size_t override {
        return ctrl.menus.size();
    }

    auto get_menu_label(const size_t index) -> std::string_view override {
        return ctrl.menus[index].name;
    }

    auto get_menu_value(const size_t index) -> int override {
        return int(ctrl.menus[index].index);
    }
};

auto controls_to_rows(std::vector<v4l2::Control> controls) -> std::vector<vcw::Row> {
    auto ret = std::vector<vcw::Row>();
    for(auto& ctrl : controls) {
        if(ctrl.type == v4l2::ControlType::Menu) {
            ret.emplace_back(vcw::Row::create<vcw::Label>(ctrl.name));
        }
        const auto ptr = new Control();
        ptr->ctrl      = std::move(ctrl);
        ret.emplace_back(vcw::Row::create<vcw::ControlPtr>(ptr));
    }
    ret.emplace_back(vcw::Row::create<vcw::QuitButton>());

    return ret;
}

struct UserCallbacks : public vcw::UserCallbacks {
    int                    fd;
    std::vector<vcw::Row>* rows;

    auto set_control_value(vcw::Control& control, int value) -> void override {
        auto& ctrl   = std::bit_cast<Control*>(&control)->ctrl;
        ctrl.current = value;
        v4l2::set_control(fd, ctrl.id, ctrl.current);
        if(ctrl.type == v4l2::ControlType::Bool || ctrl.type == v4l2::ControlType::Menu) {
            // reload for newly activated/inactivated controls;
            *rows = controls_to_rows(v4l2::query_controls(fd));
        }
    }
};

auto main(const int argc, const char* argv[]) -> int {
    ensure(argc == 2);

    const auto fd = open(argv[1], O_RDWR);
    ensure(fd >= 0);
    auto rows = controls_to_rows(v4l2::query_controls(fd));

    auto user_callbacks  = std::shared_ptr<UserCallbacks>(new UserCallbacks());
    user_callbacks->fd   = fd;
    user_callbacks->rows = &rows;

    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<vcw::Callbacks>(new vcw::Callbacks(rows, user_callbacks));
    runner.push_task(app.run(), app.open_window({.title = "v4l2-wlctl", .manual_refresh = true}, std::move(cbs)));
    runner.run();
    return 0;
}
