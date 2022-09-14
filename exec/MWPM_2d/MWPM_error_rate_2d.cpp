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
    double px, py, pz, pm;
    if(mode == 0) {
        px = p_eff / 3, py = p_eff / 3, pz = p_eff / 3, pm = 0;
    } else if(mode == 1) {
        double p_i = 1 - sqrt(1 - p_eff);
        px = p_i * (1 - p_i), py = p_i * p_i, pz = p_i * (1 - p_i), pm = 0;
    }
    auto error_model = make_shared<Err::ErrorModel::IIDError>(px, py, pz, pm);
    auto code = Err::PlanarSurfaceCode(d, error_model);
    auto decoder = Dc::Matching::StandardMWPMDecoder(px, py, pz, pm, false, 1, code.get_shape());
    
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
const int mode = 0;

int main() {
    /*
    At d = 11, 10000 samples:
    p     | T      | p_L 
    0.01  | 297ms  | 0.0394
    0.02  | 591ms  | 0.2574
    0.03  | 872ms  | 0.4976
    0.04  | 1.08s  | 0.6427
    0.05  | 1.24s  | 0.7101

    At d = 27, 10000 samples:
    p     | T      | p_L 
    0.01  | 5.95s  | 0.0032
    0.02  | 19.7s  | 0.2379
    0.03  | 35.6s  | 0.6294
    0.04  | 48.1s  | 0.7359
    0.05  | 56.7s  | 0.7415
    */


    auto path = std::filesystem::path(PROJECT_ROOT_PATH) / "exec/MWPM_2d/out/";
    string file_name;
    if(mode == 0) {
        file_name = "MWPM_2d_out_balance.txt";
    } else if(mode == 1) {
        file_name = "MWPM_2d_out_independent.txt";
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
            //chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
            vector<int> result = vector<int>(3, 0);
            #pragma omp parallel for shared(d_it, p_it, result) num_threads(NUM_THREAD)
            for(int _ = 0; _ < repeat; _++) {
                auto batch_result = test_batch(*d_it, 1.0 - pow(1.0 - (*p_it), 8.0), mode);
                result[0] += batch_result[0];
                result[1] += batch_result[1];
                result[2] += batch_result[2];
            }
            file << *d_it << " " << *p_it << " " << result[0] << " " << result[1] << " " << result[2] << " " << endl;
            //chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
            //chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
            //std::cout << "d = " << *d_it << " p = " << *p_it << "\ttime spent: " << time_span.count() << "s" << endl;
        }
    }
    file.close();
}