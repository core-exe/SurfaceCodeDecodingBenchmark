#include "iid_error.hpp"

#include <random>

namespace ErrorDynamics {
namespace ErrorModel {

std::pair<std::shared_ptr<CodeScheme::PlanarError>, std::shared_ptr<CodeScheme::PlanarSyndrome>> IIDError::generate_planar_error(CodeScheme::PlanarShape shape) {
    std::random_device random_device{};
    std::mt19937 rng_engine(random_device());
    std::discrete_distribution<> data_distribution({1-px-py-pz, px, py, pz});
    std::discrete_distribution<> measure_distribution({1-pm, pm});

    auto ret = std::make_pair(std::make_shared<CodeScheme::PlanarError>(shape.x(), shape.y()), std::make_shared<CodeScheme::PlanarSyndrome>(shape.x(), shape.y()));

    for(int i = 0; i < shape.x(); i++) {
        for(int j = 0; j < shape.y(); j++) {
            if((i + j) % 2 == 0) {
                ret.first->mult_error(CodeScheme::PlanarIndex(i, j), (Util::Pauli)data_distribution(rng_engine));
            } else {
                ret.second->change_symptom(CodeScheme::PlanarIndex(i, j), (Util::Symptom)measure_distribution(rng_engine));
            }
        }
    }

    return ret;   
} 

}}