
xcodebuild -project engine_sdk/engine_sdk.xcodeproj/ -configuration "Debug" -sdk iphonesimulator
xcodebuild -project engine_sdk/engine_sdk.xcodeproj/ -configuration "Release" -sdk iphonesimulator
xcodebuild -project engine_sdk/engine_sdk.xcodeproj/ -configuration "Debug" -sdk iphoneos
xcodebuild -project engine_sdk/engine_sdk.xcodeproj/ -configuration "Release" -sdk iphoneos

sh ./generate_fat_lib.sh
