/*
 * @Author: your name
 * @Date: 2021-11-05 14:07:35
 * @LastEditTime: 2021-11-05 22:03:16
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /joblib/src/library.cpp
 */
#include "library.h"

#include <iostream>
#include <utility>

void hello() {
    std::cout << "Hello, World!" << std::endl;
}

Parallel::Parallel(int n_jobs, std::string backend, int verbose, double timeout, std::string pre_dispatch,
                   std::string batch_size, std::string temp_folder, std::string max_nbytes, std::string mmap_mode,
                   std::string prefer, std::string require):n_jobs(n_jobs), backend(std::move(backend)), verbose(verbose), timeout(timeout),
                   pre_dispatch(pre_dispatch), batch_size(batch_size), temp_folder(std::move(temp_folder)), max_nbytes(std::move(max_nbytes)),
                   mmap_mode(mmap_mode), prefer(prefer), require(require)
{
    std::cout<<"Creating parallel task..."<<std::endl;
}
