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
#define BATCH_SIZE 40

using namespace std;
namespace Err = ErrorDynamics;
namespace Dc = Decoder;

vector<long long> test_batch(int d, double p_eff, int t_total, int mode = 0) {
    /*
    returned array:
    logical errors, # Y-error when logical error, # error when logical error,
    mode = 0: balanced error
    mode = 1: independent X/Z error
    */
    auto ret = vector<long long>(3, 0);
    double px, py, pz, pm;
    if(mode == 0) {
        px = p_eff / 3, py = p_eff / 3, pz = p_eff / 3;
        pm = px + pz;
    } else if(mode == 1) {
        double p_i = 1 - sqrt(1 - p_eff);
        px = p_i * (1 - p_i), py = p_i * p_i, pz = p_i * (1 - p_i);
        pm = px + pz;
    }
    auto error_model = make_shared<Err::ErrorModel::IIDError>(px, py, pz, pm);
    auto code = Err::PlanarSurfaceCode(d, error_model);
    auto decoder_std = Dc::Matching::StandardMWPMDecoder(px, py, pz, pm, true, t_total, code.get_shape());
    auto decoder_cs = Dc::Matching::StandardMWPMDecoder(px, py, pz, 0, false, 1, code.get_shape());
    
    for(int _ = 0; _ < BATCH_SIZE; _++) {
        code.step(t_total);
        auto data_std = code.get_data();
        auto correction_std = decoder_std(data_std);
        auto stat = data_std.second->count_errors();
        code.apply_correction(correction_std);
        
        auto correction_cs = decoder_cs(code.get_data());
        code.apply_correction(correction_cs);

        if(!code.is_correct()) {
            ret[0]++;
            ret[1] += stat[2];
            ret[2] += (stat[1] + stat[2] + stat[3]);
        }
        code.reset();
    }
    return ret;
}

const vector<int> d_list = vector<int>({7, 11, 15, 19});
const vector<double> p_list = vector<double>({
    0.0060, 0.0065, 0.0070, 0.0075, 0.0080, 0.0085, 0.0090, 0.0095, 0.0100, 0.0105, 0.0110, 0.0115, 0.0120
});
const vector<int> T_list = vector<int>({
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10
});
const int mode = 0;

int main() {
    auto path = std::filesystem::path(PROJECT_ROOT_PATH) / "exec/MWPM_3d/out/";
    string file_name;
    if(mode == 0) {
        file_name = "MWPM_3d_out_balance.txt";
    } else if(mode == 1) {
        file_name = "MWPM_3d_out_independent.txt";
    }
    ofstream file;
    file.open(path.append(file_name));

    /*
    N = 5000, NUM_THREAD = 10: ~45min
    N = 200000, NUM_THREAD = 50: ~15hrs
    */
    const int N = 200000;
    const int repeat = N / (BATCH_SIZE * T_list.size());

    file << "N" << endl;
    file << repeat * BATCH_SIZE << endl;

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

    file << "T" << endl;
    for(auto T_it = T_list.begin(); T_it != T_list.end(); T_it++) {
        file << *T_it << " ";
    }
    file << "\n";

    if(mode == 0) {
        file << "balance" << endl;
    } else if(mode == 1) {
        file << "independent" << endl;
    }
    
    for(auto d_it = d_list.begin(); d_it != d_list.end(); d_it++) {
        chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
        for(auto p_it = p_list.begin(); p_it != p_list.end(); p_it++) {
            for(auto T_it = T_list.begin(); T_it != T_list.end(); T_it++) {
                vector<long long> result = vector<long long>(3, 0);
                #pragma omp parallel for shared(d_it, p_it, result) num_threads(NUM_THREAD)
                for(int _ = 0; _ < repeat; _++) {
                    auto batch_result = test_batch(*d_it, 1.0 - pow(1.0 - (*p_it), 8.0), *T_it, mode);
                    result[0] += batch_result[0];
                    result[1] += batch_result[1];
                    result[2] += batch_result[2];
                }
                file << *d_it << " " << *p_it << " " << *T_it << " " << repeat * BATCH_SIZE - result[0] << " "  << endl;
            }
        }
        chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
        std::cout << "d = " << *d_it << "\ttime spent: " << time_span.count() << "s" << endl;
    }
    file.close();
}