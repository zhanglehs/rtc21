set PATH=%PATH%;D:\src\protobuf-3.2.0\cmake\Debug;
protoc -I=.\ --cpp_out=.\ .\proto_rtp_ext.proto
pause