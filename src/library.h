/*
 * @Author: your name
 * @Date: 2021-11-05 14:07:35
 * @LastEditTime: 2021-11-05 22:02:53
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /joblib/src/library.h
 */
#ifndef JOBLIB_LIBRARY_H
#define JOBLIB_LIBRARY_H
#include "cstdio"
#include "thread"
#include "mutex"
#include "string"

void hello();


class Parallel{
    int n_jobs;
    std::string backend;
    std::string prefer;
    std::string require;
    int verbose;
    double timeout;
    std::string pre_dispatch;
    std::string batch_size;
    std::string temp_folder;
    std::string max_nbytes;
    std::string mmap_mode;

public:

    explicit Parallel(int n_jobs=1, std::string backend="thread", int verbose=0, double timeout=60.0,
             std::string pre_dispatch="2 * n_jobs", std::string batch_size="auto", std::string temp_folder="",
             std::string max_nbytes="1M", std::string mmap_mode="r", std::string prefer="", std::string require="");
};

#endif //JOBLIB_LIBRARY_H
