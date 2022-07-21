#pragma once

#include "code_scheme.hpp"
#include "error_model.hpp"

#include <vector>
#include <utility>

namespace ErrorDynamics {

class RectangularSurfaceCode {
    private:
    int t, x, y;
    std::shared_ptr<CodeScheme::RectScheme> scheme;
    std::shared_ptr<ErrorModel::ErrorModelBase> model;
    std::shared_ptr<CodeScheme::RectSyndrome> last_syndrome;
    std::shared_ptr<CodeScheme::RectError> last_error;
    std::shared_ptr<std::vector<std::shared_ptr<CodeScheme::RectSyndrome>>> syndrome_change_list;

    public:
    RectangularSurfaceCode() = delete;
    RectangularSurfaceCode(int d, double p);
    RectangularSurfaceCode(int _x, int _y, double p);
    RectangularSurfaceCode(int d, std::shared_ptr<ErrorModel::ErrorModelBase> _model);
    RectangularSurfaceCode(int _x, int _y, std::shared_ptr<ErrorModel::ErrorModelBase> _model);

    void reset();
    void step(int dt = 1);
    void manual_step(std::shared_ptr<CodeScheme::RectError> data_error, std::shared_ptr<CodeScheme::RectSyndrome> syndrome_error);
    inline void apply_correction(std::shared_ptr<CodeScheme::RectError> correction) {
        scheme->add_data_error(correction);
    }
    
    bool is_valid() const;
    bool is_correct() const;

    inline std::pair<std::shared_ptr<std::vector<std::shared_ptr<CodeScheme::RectSyndrome>>>, 
    std::shared_ptr<CodeScheme::RectError>> get_data() const {
        return std::make_pair(syndrome_change_list, last_error);
    }

    std::shared_ptr<CodeScheme::RectSyndrome> get_syndrome() const {
        return scheme->get_syndrome();
    }

    inline const CodeScheme::RectShape get_shape() const {
        return CodeScheme::RectShape(x, y);
    }

    inline std::string to_string(bool color = false, int interval = 1) const {
        return scheme->to_string(color, interval);
    }
};

using RectData = std::pair<std::shared_ptr<std::vector<std::shared_ptr<CodeScheme::RectSyndrome>>>, 
    std::shared_ptr<CodeScheme::RectError>>;

}