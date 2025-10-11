project('simple-opengl', 'cpp', 'c',
  version: '0.1.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++23',
  ],
)

# OpenGL
gl_dep = dependency('OpenGL', required: true)
glm_dep = dependency('glm', required: true)
glfw_dep = dependency('glfw3', required: true)

stb_dep = dependency('stb', required: false)
if not stb_dep.found()
  stb_proj = subproject('stb')
  stb_dep = stb_proj.get_variable('stb_dep')
endif

assimp_dep = dependency('assimp', required: true)

glad_incdir = include_directories('contrib')

glad = static_library(
  'glad',
  'contrib/glad.c',
  include_directories: glad_incdir,
)

incdir = include_directories('include')

# C++
executable(
  'simple-opengl',
  'src/main.cpp',
  dependencies: [
    gl_dep,
    glm_dep,
    glfw_dep,
    stb_dep,
    assimp_dep,
  ],
  link_args: ['-Wl,--no-as-needed'],
  link_with: glad,
  include_directories: [incdir, glad_incdir],
  install: true,
)

# compile_commands.json
run_target('compile_commands',
  command: ['meson', 'compile_commands'],
  depends: [],
)
