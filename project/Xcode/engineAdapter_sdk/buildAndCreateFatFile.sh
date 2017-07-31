

#!/bin/bash

PROJECT_NAME="engineAdapter_sdk"
#工程的名字
MY_PROJECT_NAME="engineAdapter_sdk.xcodeproj"
#编译target的名字
MY_TARGET_NAME="engineAdapter_sdk"
#LIB名字
MY_STATIC_LIB="lib${PROJECT_NAME}.a"

#最终静态库路径
MY_FINAL_BUILD_PATH=''
MY_FINAL_BUILD_PATH_DEBUG="../out/Debug/"
MY_FINAL_BUILD_PATH_RELEASE="../out/Release/"
#最终静态库名字
MY_FINAL_STATIC_LIB="libengineAdapter_sdk.a"

#编译路径
if [ $# -eq 1 ]; then
	if [ "$1" == "Release" ]; then
		MY_FINAL_BUILD_PATH=${MY_FINAL_BUILD_PATH_RELEASE}
		# armv7 armv7s
		MY_ARMV7_BUILD_PATH='../temp/armv7'
		MY_CURRENT_BUILD_PATH="${MY_ARMV7_BUILD_PATH}"

		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Release'  -sdk 'iphoneos10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='armv7 armv7s'  VALID_ARCHS='armv7 armv7s' IPHONEOS_DEPLOYMENT_TARGET='5.0' GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build

		# arm64  代码未修改所以报错
		MY_ARM64_BUILD_PATH='../temp/arm64'
		MY_CURRENT_BUILD_PATH="${MY_ARM64_BUILD_PATH}"

		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Release' -sdk 'iphoneos10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='arm64'  VALID_ARCHS='arm64' IPHONEOS_DEPLOYMENT_TARGET='7.0'  GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build

		# i386
		MY_I386_BUILD_PATH='../temp/i386'
		MY_CURRENT_BUILD_PATH="${MY_I386_BUILD_PATH}"

		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Release' -sdk 'iphonesimulator10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='i386' VALID_ARCHS='i386' IPHONEOS_DEPLOYMENT_TARGET='5.0' GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build

		# x86_64 代码未兼容所以报错
		MY_X86_64_BUILD_PATH='../temp/x86_64'
		MY_CURRENT_BUILD_PATH="${MY_X86_64_BUILD_PATH}"
		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Release' -sdk 'iphonesimulator10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='x86_64' VALID_ARCHS='x86_64' IPHONEOS_DEPLOYMENT_TARGET='7.0' GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build
		
	elif [ "$1" == "Debug" ]; then
		MY_FINAL_BUILD_PATH=${MY_FINAL_BUILD_PATH_DEBUG}
# 		armv7 armv7s
		MY_ARMV7_BUILD_PATH='../temp/armv7'
		MY_CURRENT_BUILD_PATH="${MY_ARMV7_BUILD_PATH}"
		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Debug'  -sdk 'iphoneos10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='armv7 armv7s'  VALID_ARCHS='armv7 armv7s' IPHONEOS_DEPLOYMENT_TARGET='5.0' GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build
		
		# arm64  代码未修改所以报错
		MY_ARM64_BUILD_PATH='../temp/arm64'
		MY_CURRENT_BUILD_PATH="${MY_ARM64_BUILD_PATH}"
		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Debug' -sdk 'iphoneos10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='arm64' VALID_ARCHS='arm64' IPHONEOS_DEPLOYMENT_TARGET='7.0'  GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build

		# i386
		MY_I386_BUILD_PATH='../temp/i386'
		MY_CURRENT_BUILD_PATH="${MY_I386_BUILD_PATH}"
		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Debug' -sdk 'iphonesimulator10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='i386' VALID_ARCHS='i386' IPHONEOS_DEPLOYMENT_TARGET='5.0' GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build

		# x86_64 代码未兼容所以报错
		MY_X86_64_BUILD_PATH='../temp/x86_64'
		MY_CURRENT_BUILD_PATH="${MY_X86_64_BUILD_PATH}"
		xcodebuild -scheme "${PROJECT_NAME}"  -workspace "${PROJECT_NAME}.xcworkspace" -configuration 'Debug' -sdk 'iphonesimulator10.2' CONFIGURATION_BUILD_DIR="${MY_CURRENT_BUILD_PATH}" ARCHS='x86_64' VALID_ARCHS='x86_64' IPHONEOS_DEPLOYMENT_TARGET='7.0' GCC_PREPROCESSOR_DEFINITIONS="GITCOMMIT=$(git rev-parse --short HEAD)" clean build
	else
		echo "Parame is [Release] or [Debug]"
	fi
	#合并不同版本的编译库
	mkdir ${MY_FINAL_BUILD_PATH}
	lipo -create "${MY_ARMV7_BUILD_PATH}/${MY_STATIC_LIB}" "${MY_ARM64_BUILD_PATH}/${MY_STATIC_LIB}" "${MY_I386_BUILD_PATH}/${MY_STATIC_LIB}" "${MY_X86_64_BUILD_PATH}/${MY_STATIC_LIB}" -output "${MY_FINAL_BUILD_PATH}${MY_FINAL_STATIC_LIB}"
else
	echo "parame too much, should == 1 Release or Debug"
fi

if [ -d "build" ]; then
	rm -rf build
fi

if [ -d "../temp" ]; then
	rm -rf ../temp
fi

# open "${MY_FINAL_BUILD_PATH}"
