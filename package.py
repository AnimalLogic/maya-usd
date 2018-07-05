# -*- coding: utf-8 -*-
name = 'AL_USDMaya'

version = '0.28.4'

authors = ['eoinm']

description = 'USD to Maya Bridge'

private_build_requires = [
    'AL_CMakeLib',
    'cmake-2.8+',
    'AL_CMakeLibPython-6.0.9+<7',
    'gcc-4.8',
    'AL_MTypeId-1.41+',
    'gdb-7.10',
    'doxygen-1',
    'AL_USDSchemas-0.37.1+<1', # For the SdfMetadata only
    'AL_maya_startup-1+'      # To help cmake find "maya" executable
]

requires = [
    'usdBase-0.8.5',
    'usdImaging-0.8.5',
    'glew-2.0',
    'googletest',
    'python-2.7+<3',
    'doubleConversion-1',
    'stdlib-4.8',
    'zlib-1.2',
    'cppunit-1.12+<2',
    'AL_CMakeLibGitHub-0.1.0+<1',
    '~AL_USDSchemas-0.39+<1', # For the SdfMetadata only
    'AL_boost-1.55',
    'AL_boost_python-1.55',
]

variants = [
    ['CentOS-6.2+<7', 'mayaDevKit-2017.0'],
    ['CentOS-6.2+<7', 'mayaDevKit-2018.0'],
    ['CentOS-6.2+<7', 'mayaDevKit-2019.b92']
]

help = [['API', '$BROWSER http://github.al.com.au/pages/documentation/AL_USDMaya']]

def commands():
    prependenv('PATH', '{root}/bin')
    prependenv('PYTHONPATH', '{root}/lib/python')
    prependenv('LD_LIBRARY_PATH', '{root}/lib')
    prependenv('MAYA_PLUG_IN_PATH', '{root}/plugin')
    prependenv('MAYA_SCRIPT_PATH', '{root}/lib:{root}/share/usd/plugins/usdMaya/resources')
    prependenv('PXR_PLUGINPATH', '{root}/share/usd/plugins')
    prependenv('CMAKE_MODULE_PATH', '{root}/cmake')
    prependenv('DOXYGEN_TAGFILES', '{root}/doc/AL_USDMaya.tag=http://github.al.com.au/pages/documentation/AL_USDMaya')
    setenv('AL_USDMAYA_AUTOLOAD_MEL', '{root}/mel')
    prependenv('AL_MAYA_AUTO_LOADVERSIONEDTOOL', 'al_usdmaya_autoload')
    
    # workaround for tbb-4.4 warnings
    # maya initializes tbb before usd with a value which seems to be (available cores - 3)
    # usd then wants to initialize tbb with (available cores) which leads to the warnings
    # manually set usd's limit to this empirically observed value
    env.PXR_WORK_THREAD_LIMIT='-3'


