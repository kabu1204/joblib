#!/bin/bash
###
 # @Author: your name
 # @Date: 2021-11-05 22:21:30
 # @LastEditTime: 2021-11-05 23:14:04
 # @LastEditors: Please set LastEditors
 # @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 # @FilePath: /joblib/cmake-build.sh
###
mode="Release"
if [ "$1" == "" -o "$1" == "release" ]
then
    echo "Build in Release type 进行优化，提高速度，不包含调试信息"
    mode="Release"
elif [ $1 == "debug" ]
then
    echo "Build in Debug type - 禁用优化，附加调试信息"
    mode="Debug"
elif [ $1 == "reldeb" ]
then
    echo "Build in RelWithDebInfo type - 进行优化，提高速度，附加调试信息"
    mode="RelWithDebInfo"
else
    echo "Usage: $0 [release | debug | reldeb]"
    exit
fi
mode_l="$(tr [A-Z] [a-z] <<< "$mode")"
path_to_build="cmake-build-${mode_l}"
cmake . -B $path_to_build -DCMAKE_BUILD_TYPE=$mode
cmake --build $path_to_build --target test