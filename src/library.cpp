#include "library.h"
#include <iostream>
#include <utility>
INITIALIZE_EASYLOGGINGPP

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
