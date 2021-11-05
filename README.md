<!--
 * @Author: your name
 * @Date: 2021-11-05 14:22:42
 * @LastEditTime: 2021-11-05 23:34:09
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /joblib/README.md
-->
# C++ version of python module 'joblib'

## Build
### CMake
Generate and build:
```shell
chmod +x cmake-build.sh
./cmake-build.sh debug  # or release/reldeb
```
shared libraries and executable files is in cmake-build-debug

### xmake
Generate and build:
```shell
xmake f -m debug    # or release/reldeb
xmake
```
shared libraries and executable files is in xmake-build