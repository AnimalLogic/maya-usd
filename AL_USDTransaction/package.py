# -*- coding: utf-8 -*-
name = 'AL_USDTransaction'

version = '0.3.2'

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
  ['usdBase-0.20.2'],
]

def commands():
    prependenv('LD_LIBRARY_PATH', '{root}/lib')
    prependenv('CMAKE_MODULE_PATH', '{root}/cmake')
    prependenv('PYTHONPATH', '{root}/lib/python')
    prependenv('PXR_PLUGINPATH_NAME', '{root}/lib/usd')

with scope('config') as config:
    config.sub_repositories = ['../../maya-usd']
    import os
    '''
    MAYA_USD_GI_TAG:
    Tag (Branch name, Commit ID or tag) see https://cmake.org/cmake/help/v3.11/module/FetchContent.html "GIT_TAG"
    Used to specify what branch/commit/tag of maya-usd to checkout in non-interactive mode
    Leave empty to use default (master) HEAD
    '''
    os.environ['MAYA_USD_GIT_TAG']= ''