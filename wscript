srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check_cfg(package='glib-2.0', args='--cflags --libs', mandatory=True)
    conf.check_cc(lib='m', uselib_store='M', mandatory=True)
    conf.check_cc(lib='crypt', uselib_store='CRYPT', mandatory=True)

def build(bld):
    bld.add_subdirs('src')
