project('v4l2-wlctl', ['c', 'cpp'], version : '2.0.0', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++20'])

subdir('src/gawl')

wlctl_file = files(
  'src/main.cpp',
  'src/v4l2.cpp',
  'src/window.cpp',
) + gawl_core_files + gawl_textrender_files + gawl_fc_files + gawl_no_touch_callbacks_file

wlctl_deps = gawl_core_deps + gawl_textrender_deps + gawl_fc_deps

oneshot_files = files(
  'src/oneshot.cpp',
  'src/v4l2.cpp',
)

executable('v4l2-wlctl', wlctl_file,
            dependencies : wlctl_deps,
            install : true,
)

executable('v4l2-wlctl-oneshot', oneshot_files,
            install : true,
)
