project('simple-opengl', 'cpp', 'c',
  version: '0.1.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++23',
  ],
)

stb_dep = dependency('stb', required: false)
if not stb_dep.found()
  stb_proj = subproject('stb')
  stb_dep = stb_proj.get_variable('stb_dep')
endif

perlin_noise_dep = dependency('PerlinNoise', required: false)
if not perlin_noise_dep.found()
  perlin_noise_proj = subproject('PerlinNoise')
  perlin_noise_dep = perlin_noise_proj.get_variable('perlin_noise_dep')
endif

dependencies = [
  dependency('OpenGL', required: true),
  dependency('epoxy', required: true),
  dependency('glm', required: true),
  dependency('glfw3', required: true),
  stb_dep,
  dependency('assimp', required: true),
  perlin_noise_dep,
]

incdir = include_directories('include')

includes = [
  incdir,
]

# C++
executable(
  'simple-opengl',
  sources: [
    'src/main.cpp',
    'src/core/application.cpp',
    'src/core/input_handler.cpp',
    'src/graphics/graphics_renderer.cpp',
    'src/graphics/cloud.cpp',
    'src/graphics/terrain_mesh.cpp',
    'src/graphics/particles/particle_system.cpp',
    'src/graphics/particles/particle_factory.cpp',
  ],
  dependencies: dependencies,
  link_args: ['-Wl,--no-as-needed'],
  include_directories: includes,
  install: true,
)

# compile_commands.json
run_target('compile_commands',
  command: ['meson', 'compile_commands'],
  depends: [],
)
