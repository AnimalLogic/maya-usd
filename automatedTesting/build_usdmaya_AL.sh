 #!/usr/bin/env bash
set -e

mkdir -p $TMP_DIR


MAYA_DEVKIT_LOCATION=$MAYA_LOCATION  # Our Docker script defines $MAYA_LOCATION as this - should probably change
MAYA_LOCATION=/usr/autodesk/maya2019

MAYAUSD_INSTALL_LOCATION=$BUILD_DIR/mayaUSD


#----------------------------------------------
# We're building the minimal setup that AL use internally here... (see --build-args)

cd $TMP_DIR &&\
    rm -Rf build &&\
    mkdir build &&\
    cd build  &&\
    python $TMP_DIR/maya-usd/build.py \
      --build-args=-DBUILD_USDMAYA_PXR_TRANSLATORS=0,-DBUILD_PXR_PLUGIN=0,-DBUILD_ADSK_PLUGIN=0,DPXR_STRICT_BUILD_MODE=1 \
      --stages clean,configure,build,install \
      --verbosity 5 \
      --build-location $TMP_DIR/build/ \
      --install-location $MAYAUSD_INSTALL_LOCATION \
      --pxrusd-location $BUILD_DIR \
      --devkit-location $MAYA_DEVKIT_LOCATION \
      --maya-location $MAYA_LOCATION \
      .. 
    cd $TMP_DIR/build
    ctest -V -R "Test"	#Lucky pattern that happens to run only the AL tests
    cd ..
    rm -rf build


