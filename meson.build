project('sadx64', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

yasm_src = ['src/sad.s']

yasm = find_program('yasm')

gen = generator(yasm,
  output  : '@BASENAME@.obj',
  arguments : ['-g', 'stabs', '-f', 'elf64', '@INPUT@', '-o', '@OUTPUT@'])

yasm_objs = gen.process(yasm_src)

src_c = ['src/main.c', 'src/sad-test.c']

src_c += yasm_objs
exe = executable('sadx64',
           sources: src_c,
           install : true)
