#include "error_dynamics.hpp"
#include "decoder.hpp"

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <utility>
#include <fstream>
#include <filesystem>
#include <omp.h>

#define NUM_THREAD 50
#define BATCH_SIZE 1000

using namespace std;
namespace Err = ErrorDynamics;
namespace Dc = Decoder;

vector<int> test_batch(int d, double p_eff, int mode = 0) {
    /*
    returned array:
    logical errors, # Y-error when logical error, # error when logical error,
    mode = 0: balanced error
    mode = 1: independent X/Z error
    */
    auto ret = vector<int>(3, 0);
    shared_ptr<Err::ErrorModel::ErrorModelBase> error_model;
    if(mode == 0)
        error_model = static_pointer_cast<Err::ErrorModel::ErrorModelBase>(make_shared<Err::ErrorModel::IIDError>(p_eff / 3, p_eff / 3, p_eff / 3, 0));
    else if(mode == 1) {
        double p_independent = sqrt(1 + p_eff) - 1;
        error_model = static_pointer_cast<Err::ErrorModel::ErrorModelBase>(make_shared<Err::ErrorModel::IIDError>(p_independent,  pow(p_independent, 2.0), p_independent, 0));
    }
    auto code = Err::RectangularSurfaceCode(d, error_model);
    auto decoder = Dc::Matching::StandardMWPMDecoder(p_eff, p_eff, p_eff, 0, false, code.get_shape());
    
    for(int _ = 0; _ < BATCH_SIZE; _++) {
        code.step(1);
        auto data = code.get_data();
        auto correction = decoder(data);
        auto stat = data.second->count_errors();
        
        code.apply_correction(correction);
        auto corrected = data.second * correction;
        if(!corrected->is_correct()) {
            ret[0]++;
            ret[1] += stat[2];
            ret[2] += (stat[1] + stat[2] + stat[3]);
        }
        code.reset();
    }
    return ret;
}

const vector<int> d_list = vector<int>({7, 11, 15, 19, 23, 27});
const vector<double> p_list = vector<double>({
    0.001, 0.002, 0.003, 0.004, 0.005,
    0.006, 0.007, 0.008, 0.009, 0.010,
    0.011, 0.012, 0.013, 0.014, 0.015,
    0.016, 0.017, 0.018, 0.019, 0.020,
    0.022, 0.024, 0.026, 0.028, 0.030,
    0.032, 0.034, 0.036, 0.038, 0.040,
    0.042, 0.044, 0.046, 0.048, 0.050
});
const int mode = 1;

int main() {
    /*
    At d = 11, 10000 samples:
    p     | T      | p_L 
    0.01  | 3.65s  | 0.0394
    0.02  | 6.92s  | 0.2574
    0.03  | 9.77s  | 0.4976
    0.04  | 11.9s  | 0.6427
    0.05  | 13.5s  | 0.7101

    At d = 27, 10000 samples:
    p     | T      | p_L 
    0.01  | 59.5s  | 0.0032
    0.02  | 217s   | 0.2379
    0.03  | 416s   | 0.6294M
    0.04  | 595s   | 0.7359
    0.05  | 722s   | 0.7415
    */


    auto path = std::filesystem::path(PROJECT_ROOT_PATH) / "exec/MWPM_2d/out/";
    string file_name;
    if(mode == 0) {
        file_name = "MWPM_2d_out_balance_.txt";
    } else if(mode == 1) {
        file_name = "MWPM_2d_out_independent_.txt";
    }
    ofstream file;
    file.open(path.append(file_name));

    int N = 1000000;
    file << "N" << endl;
    file << N << endl;

    file << "d" << endl;
    for(auto d_it = d_list.begin(); d_it != d_list.end(); d_it++) {
        file << *d_it << " ";
    }
    file << "\n";

    file << "p" << endl;
    for(auto p_it = p_list.begin(); p_it != p_list.end(); p_it++) {
        file << *p_it << " ";
    }
    file << "\n";

    if(mode == 0) {
        file << "balance" << endl;
    } else if(mode == 1) {
        file << "independent" << endl;
    }
    
    const int repeat = N / BATCH_SIZE;
    for(auto d_it = d_list.begin(); d_it != d_list.end(); d_it++) {
        for(auto p_it = p_list.begin(); p_it != p_list.end(); p_it++) {
            vector<int> result = vector<int>(3, 0);
            #pragma omp parallel for shared(d_it, p_it, result) num_threads(NUM_THREAD)
            for(int _ = 0; _ < repeat; _++) {
                auto batch_result = test_batch(*d_it, 1.0 - pow(1.0 - (*p_it), 8.0), mode);
                result[0] += batch_result[0];
                result[1] += batch_result[1];
                result[2] += batch_result[2];
            }
            file << *d_it << " " << *p_it << " " << result[0] << " " << result[1] << " " << result[2] << " " << endl;
        }
    }
    file.close();
}