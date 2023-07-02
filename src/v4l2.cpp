#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "assert.hpp"
#include "v4l2.hpp"

namespace v4l2 {
template <class... Args>
auto xioctl(const int fd, const int request, Args&&... args) -> int {
    int r;
    do {
        r = ioctl(fd, request, std::forward<Args>(args)...);
    } while(r == -1 && errno == EINTR);
    return r;
}

auto enumerate_menu(const int fd, const v4l2_queryctrl& queryctrl) -> std::vector<ControlMenu> {
    auto ret       = std::vector<ControlMenu>();
    auto querymenu = v4l2_querymenu();

    querymenu.id = queryctrl.id;

    for(querymenu.index = queryctrl.minimum; querymenu.index <= (uint32_t)queryctrl.maximum; querymenu.index += 1) {
        if(xioctl(fd, VIDIOC_QUERYMENU, &querymenu) == 0) {
            auto menu = ControlMenu();
            memcpy(menu.name, querymenu.name, 32);
            menu.index = querymenu.index;
            ret.emplace_back(menu);
        }
    }

    return ret;
}

#define APPEND(def)      \
    if(flags & def) {    \
        ret += #def ","; \
    }
auto control_flags_to_str(const uint32_t flags) -> std::string {
    auto ret = std::string();
    APPEND(V4L2_CTRL_FLAG_DISABLED);
    APPEND(V4L2_CTRL_FLAG_GRABBED);
    APPEND(V4L2_CTRL_FLAG_READ_ONLY);
    APPEND(V4L2_CTRL_FLAG_UPDATE);
    APPEND(V4L2_CTRL_FLAG_INACTIVE);
    APPEND(V4L2_CTRL_FLAG_SLIDER);
    APPEND(V4L2_CTRL_FLAG_WRITE_ONLY);
    APPEND(V4L2_CTRL_FLAG_VOLATILE);
    APPEND(V4L2_CTRL_FLAG_HAS_PAYLOAD);
    APPEND(V4L2_CTRL_FLAG_EXECUTE_ON_WRITE);
    APPEND(V4L2_CTRL_FLAG_MODIFY_LAYOUT);
    APPEND(V4L2_CTRL_FLAG_DYNAMIC_ARRAY);
    return ret;
}

auto query_class_controls(const int fd, const uint32_t control_class, std::vector<Control>& ret) -> void {
    auto queryctrl = v4l2_queryctrl();

    queryctrl.id = control_class | V4L2_CTRL_FLAG_NEXT_CTRL;

    while(xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl) == 0) {
        if(V4L2_CTRL_ID2CLASS(queryctrl.id) != control_class) {
            break;
        }
        if(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
            continue;
        }

        do {
            auto type = ControlType();
            switch(queryctrl.type) {
            case V4L2_CTRL_TYPE_INTEGER:
                type = ControlType::Int;
                break;
            case V4L2_CTRL_TYPE_BOOLEAN:
                type = ControlType::Bool;
                break;
            case V4L2_CTRL_TYPE_MENU:
                type = ControlType::Menu;
                break;
            default:
                continue;
            }

            auto control = Control{
                .id       = queryctrl.id,
                .type     = type,
                .name     = {},
                .max      = queryctrl.maximum,
                .min      = queryctrl.minimum,
                .step     = queryctrl.step,
                .current  = get_control(fd, queryctrl.id),
                .menus    = {},
                .ro       = bool(queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY),
                .inactive = bool(queryctrl.flags & V4L2_CTRL_FLAG_INACTIVE),
            };

            memcpy(control.name, queryctrl.name, 32);

            if(queryctrl.type == V4L2_CTRL_TYPE_MENU) {
                control.menus = enumerate_menu(fd, queryctrl);
            }

            ret.emplace_back(control);

        } while(0);
        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}

auto query_controls(const int fd) -> std::vector<Control> {
    auto ret = std::vector<Control>();
    query_class_controls(fd, V4L2_CTRL_CLASS_USER, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_CODEC, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_CAMERA, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_FM_TX, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_FLASH, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_JPEG, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_IMAGE_SOURCE, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_IMAGE_PROC, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_DV, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_FM_RX, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_RF_TUNER, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_DETECT, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_CODEC_STATELESS, ret);
    query_class_controls(fd, V4L2_CTRL_CLASS_COLORIMETRY, ret);
    return ret;
}

auto get_control(const int fd, const uint32_t id) -> int32_t {
    auto control = v4l2_control();
    control.id   = id;

    DYN_ASSERT(xioctl(fd, VIDIOC_G_CTRL, &control) == 0);

    return control.value;
}

auto set_control(const int fd, const uint32_t id, const int32_t value) -> void {
    auto control  = v4l2_control();
    control.id    = id;
    control.value = value;

    DYN_ASSERT(xioctl(fd, VIDIOC_S_CTRL, &control) == 0);
}
} // namespace v4l2
