#!/bin/bash

mkdir -p /opt/soft

cp -f ../../src/liblive_stream_sdk.so /opt
cp -f ../../../../live_av_thirdparty/protobuf/linux/libprotobuf.so /opt

echo "/opt" > /etc/ld.so.conf.d/enginesdk.conf

ldconfig

