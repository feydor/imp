project('sadx64', 'c',
    version : '0.1.0',
    default_options : ['warning_level=3', 'c_link_args=-lasan', 'c_std=c11'])

# dependency resolution
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)
libsaru_buf_dep = dependency(
    'libsaru-buf',
    fallback: ['saru-buf', 'libsaru_buf_dep'],
    version: '>=0.0.1',
)
unity_dep = dependency(
    'Unity',
    fallback: ['Unity', 'unity_dep'],
)

# assembly compilation
yasm = find_program('yasm')
yasm_src = ['src/sad.s']
gen = generator(yasm,
    output  : '@BASENAME@.obj',
    arguments : ['-g', 'stabs', '-f', 'elf64', '@INPUT@', '-o', '@OUTPUT@'])
yasm_objs = gen.process(yasm_src)

# main program compilation
src_c = ['src/main.c', 'src/sad-test.c', 'src/sad.c', 'src/bmp.c', 'src/imageio.c', 'src/imagehandler.c', 'src/imageproc.c']
incl_dir = include_directories('include')
deps = [math_dep, libsaru_buf_dep]
src_c += yasm_objs

exe = executable('sadx64',
    sources: src_c,
    include_directories: incl_dir,
    dependencies: deps,
    c_args: ['-Wall'],
    install : true
)

# unit tests
imageproc_test = executable('imageproc-test',
    ['test/imageproc.c', 'src/imageproc.c', 'src/imageio.c', 'src/bmp.c'],
    include_directories: incl_dir,
    dependencies: [unity_dep, math_dep])
test('unittests imageproc', imageproc_test)
