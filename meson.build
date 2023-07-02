project('v4l2-wlctl', 'cpp', version : '1.0.0', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++20'])

deps = [
  dependency('gawl', version : ['>=2.6', '<2.7']),
  dependency('gawl-fc'),
]

executable('v4l2-wlctl', files('src/main.cpp', 'src/v4l2.cpp'),
            dependencies : deps,
            install : true)

executable('v4l2-wlctl-oneshot', files('src/oneshot.cpp', 'src/v4l2.cpp'),
            dependencies : [],
            install : true)
