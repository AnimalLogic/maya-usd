# -*- coding: utf-8 -*-
name = 'AL_USDTransaction'

version = '0.3.1'

private_build_requires = [
    'cmake-2.8+',
    'gcc-6.3.1+<7',
    'googletest',
    'tbb',
]

requires = [
    'AL_boost_python-1.66',
    'python-2.7',
]

variants = [
  ['usdBase-0.19.11'],
]

def commands():
    prependenv('LD_LIBRARY_PATH', '{root}/lib')
    prependenv('CMAKE_MODULE_PATH', '{root}/cmake')
    prependenv('PYTHONPATH', '{root}/lib/python')
    prependenv('PXR_PLUGINPATH_NAME', '{root}/lib/usd')
