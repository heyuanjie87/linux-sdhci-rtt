# RT-Thread building script for component

from building import *

cwd     = GetCurrentDir()
src     = []
CPPPATH = [cwd + '/include']

src += Glob('*.c')
src += Glob('port/*.c')

if GetDepend('SDHCI_PLATFORM_CVITEK'):
    src += Glob('platform/cvitek/*.c')
    if GetDepend('SDHCI_PLATFORM_CVITEK_USING_EXAMPLE'):
        src += Glob('example/cvitek-drvinit.c')

group = DefineGroup('Drivers', src, depend = ['PKG_USING_SDHCI'], CPPPATH = CPPPATH)

objs = [group]

Return('objs')
