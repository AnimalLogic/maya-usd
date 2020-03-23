# -*- coding: utf-8 -*-
name = 'AL_USDUtils'

version = '1.0.1'

private_build_requires = [
    'AL_CMakeLib',
    'cmake-2.8+',
    'gcc-6.3.1',
    'googletest-1.8',
    'tbb-4.4+',
]

requires = [
    'AL_boost-1.66',
    'AL_boost_python-1.66',
    'python-2.7',
    'CentOS-7.4+<8',
]

variants = \
    [['usdBase-0.19.11'],['usdBase-0.20.2']]

def commands():
    prependenv('LD_LIBRARY_PATH', '{root}/lib')
    prependenv('CMAKE_MODULE_PATH', '{root}/cmake')
    prependenv('PYTHONPATH', '{root}/lib/python')
