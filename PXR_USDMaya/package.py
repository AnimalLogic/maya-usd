# -*- coding: utf-8 -*-

name = 'PXR_USDMaya'

version = '0.19.11.1'

private_build_requires = [
    'cmake-3',
    'usdBase-0.19.11',
    'AL_maya_startup-1+',
    'gcc-6.3.1',
    'glew-2.0',
    'python-2.7'
]

requires = [
    'CentOS-7.4+<8',
    'AL_boost-1.66.0',
    'AL_boost_python-1.66.0',
    'usdImaging-0.19.11',
    'tbb'
]

variants = [
    ['mayaDevKit-2019.0'],
    ['mayaDevKit-2020.0']
]

def commands():
    prependenv('PATH', '{root}/src')
    prependenv('PYTHONPATH', '{root}/lib/python')
    prependenv('LD_LIBRARY_PATH', '{root}/third_party/maya/lib')
    prependenv('MAYA_PLUG_IN_PATH', '{root}/third_party/maya/plugin')
    prependenv('MAYA_SCRIPT_PATH', '{root}/lib:{root}/third_party/maya/lib/usd/usdMaya/resources')
    prependenv('CMAKE_MODULE_PATH', '{root}/cmake')
    prependenv('PXR_PLUGINPATH_NAME', '{root}/third_party/maya/plugin/pxrUsdPreviewSurface/resources')
    
    # workaround for tbb-4.4 warnings
    # maya initializes tbb before usd with a value which seems to be (available cores - 3)
    # usd then wants to initialize tbb with (available cores) which leads to the warnings
    # manually set usd's limit to this empirically observed value
    env.PXR_WORK_THREAD_LIMIT = '-3'
