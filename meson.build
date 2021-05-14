project('sadx64', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

yasm_src = ['src/sad.s']

yasm = find_program('yasm')
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)
libsaru_buf_dep = dependency(
  'libsaru-buf',
  fallback: ['saru-buf', 'libsaru_buf_dep'],
  version: '>=0.0.1',
  )

gen = generator(yasm,
  output  : '@BASENAME@.obj',
  arguments : ['-g', 'stabs', '-f', 'elf64', '@INPUT@', '-o', '@OUTPUT@'])

yasm_objs = gen.process(yasm_src)

src_c = ['src/main.c', 'src/sad-test.c', 'src/sad.c', 'src/bmp.c']
incl_dir = include_directories('include')
deps = [math_dep, libsaru_buf_dep]

src_c += yasm_objs
exe = executable('sadx64',
           sources: src_c,
           include_directories: incl_dir,
           dependencies: deps,
           install : true)
