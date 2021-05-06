project('bmpx64', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

# gather tools and compilers
cl = find_program('cl') # MSVC C compiler
rc = find_program('rc') # MSVC Resource Compiler

# compile the resource file (menus, icons, bmps)
resources = ['res/res1.rc']

res_gen = generator(rc,
  output :'@BASENAME@.RES',
  arguments: ['/v', '/r', '@INPUT@'])
res_obj = gen.process(resources)

# gather source and header files
src = ['src/main.c', 'src/controls.c', 'src/io.c']
incl_dir = include_directories('include')

src += res_obj

exe = executable('sadx64',
           sources: src,
           include_directories: incl_dir,
           install : true)
