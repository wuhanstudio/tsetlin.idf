from building import *
import rtconfig

# get current directory
cwd     = GetCurrentDir()

# The set of source files associated with this SConscript file.
src     = Glob('tsetlin/*.c')
src    += Glob('random/*.c')
src    += Glob('protobuf/*.c')
src    += Glob('protobuf-c/*.c')

path    = [cwd]
path   += [cwd + '/platforms/rt-thread']
path   += [cwd + '/protobuf']
path   += [cwd + '/tsetlin']
path   += [cwd + '/random']
path   += [cwd + '/utils']

# MNIST Examples
if GetDepend('LIME_TM_USING_MNIST_EXAMPLE'):
    path   += [cwd + '/mnist']
    src    += Glob('mnist/*.c')
    src    += Glob('platforms/rt-thread/*.c')

LOCAL_CCFLAGS = ''

group = DefineGroup('lime-tm', src, depend = ['PKG_USING_LIME_TM'], CPPPATH = path, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

Return('group')