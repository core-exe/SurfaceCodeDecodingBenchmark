#pragma once

#include "code_scheme.hpp"
#include "error_model.hpp"

#include <vector>
#include <utility>

namespace ErrorDynamics {

class PlanarSurfaceCode {
    private:
    int t, x, y;
    std::shared_ptr<CodeScheme::PlanarScheme> scheme;
    std::shared_ptr<ErrorModel::ErrorModelBase> model;
    std::shared_ptr<CodeScheme::PlanarSyndrome> last_syndrome;
    std::shared_ptr<CodeScheme::PlanarError> last_error;
    std::shared_ptr<std::vector<std::shared_ptr<CodeScheme::PlanarSyndrome>>> syndrome_change_list;

    public:
    PlanarSurfaceCode() = delete;
    PlanarSurfaceCode(int d, double p);
    PlanarSurfaceCode(int _x, int _y, double p);
    PlanarSurfaceCode(int d, std::shared_ptr<ErrorModel::ErrorModelBase> _model);
    PlanarSurfaceCode(int _x, int _y, std::shared_ptr<ErrorModel::ErrorModelBase> _model);

    void reset();
    void step(int dt = 1);
    void manual_step(std::shared_ptr<CodeScheme::PlanarError> data_error, std::shared_ptr<CodeScheme::PlanarSyndrome> syndrome_error);
    inline void apply_correction(std::shared_ptr<CodeScheme::PlanarError> correction) {
        scheme->add_data_error(correction);
    }
    
    inline bool is_valid() const { return scheme->is_valid(); }
    bool is_correct() const { return scheme->is_correct(); }

    inline std::pair<std::shared_ptr<std::vector<std::shared_ptr<CodeScheme::PlanarSyndrome>>>, 
    std::shared_ptr<CodeScheme::PlanarError>> get_data() const {
        using namespace std;
        auto syndrome_list = make_shared<vector<shared_ptr<CodeScheme::PlanarSyndrome>>>(0);
        for(auto it = syndrome_change_list->begin(); it != syndrome_change_list->end(); it++) {
            syndrome_list->push_back(make_shared<CodeScheme::PlanarSyndrome>(*(*it)));
        }
        return std::make_pair(syndrome_list, make_shared<CodeScheme::PlanarError>(*last_error));
    }

    std::shared_ptr<CodeScheme::PlanarSyndrome> get_syndrome() const {
        return scheme->get_syndrome();
    }

    inline const CodeScheme::PlanarShape get_shape() const {
        return CodeScheme::PlanarShape(x, y);
    }

    inline std::string to_string(bool color = false, int interval = 1) const {
        return scheme->to_string(color, interval);
    }

    inline std::vector<int> count_errors() const {
        return last_error->count_errors();
    }
};

using PlanarData = std::pair<std::shared_ptr<std::vector<std::shared_ptr<CodeScheme::PlanarSyndrome>>>, 
    std::shared_ptr<CodeScheme::PlanarError>>;
}