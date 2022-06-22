project('imp', 'c',
    version : '0.1.0',
    default_options : ['warning_level=3', 'c_link_args=-lasan', 'c_std=c11'])

# dependency resolution
cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)

# assembly compilation
# yasm = find_program('yasm')
# yasm_src = ['src/sad.s']
# gen = generator(yasm,
#    output  : '@BASENAME@.obj',
#    arguments : ['-g', 'stabs', '-f', 'elf64', '@INPUT@', '-o', '@OUTPUT@'])
# yasm_objs = gen.process(yasm_src)

# main program compilation
src_c = ['src/main.c']
incl_dir = include_directories('src')
deps = [math_dep]
# src_c += yasm_objs

exe = executable('imp',
    sources: src_c,
    include_directories: incl_dir,
    dependencies: deps,
    c_args: ['-Wall'],
    install : true
)

# unit tests
# imageproc_test = executable('imageproc-test',
#    ['test/imageproc.c', 'src/imageproc.c', 'src/imageio.c', 'src/bmp.c'],
#     include_directories: incl_dir,
#    dependencies: [unity_dep, math_dep])
# test('unittests imageproc', imageproc_test)
