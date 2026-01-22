from building import *
import rtconfig

# get current directory
cwd     = GetCurrentDir()

# The set of source files associated with this SConscript file.
src     = Glob('tsetlin/*.c')
src    += Glob('protobuf-c/*.c')

path    = [cwd + '/platforms/rt-thread']
path   += [cwd + '/protobuf-c']
path   += [cwd + '/tsetlin']

LOCAL_CCFLAGS = ''

group = DefineGroup('tsetlin', src, depend = ['PKG_USING_TSETLIN'], CPPPATH = path, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

Return('group')