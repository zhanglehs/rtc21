BUILD_DIR=engine_sdk/build

rm -rf out/

mkdir -p out/include/
cp ../../src/live_stream_sdk/*.h out/include/
cp ../../src/superlogic/*.h out/include/

lipo -create ${BUILD_DIR}/Debug-*/libengine_sdk.a -output out/libengine_sdk_debug.a
lipo -create ${BUILD_DIR}/Release-*/libengine_sdk.a -output out/libengine_sdk_release.a

cp ${COCOS2DX_HOME}/external/curl/prebuilt/ios/*.a out/

