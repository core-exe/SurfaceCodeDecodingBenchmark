#include "rectangular_surface_code.hpp"

namespace ErrorDynamics {

RectangularSurfaceCode::RectangularSurfaceCode(int _x, int _y, std::shared_ptr<ErrorModel::ErrorModelBase> _model) {
    t = 0;
    x = _x, y = _y;
    scheme = std::make_shared<CodeScheme::RectScheme>(x, y);
    model = _model;
    last_syndrome = std::make_shared<CodeScheme::RectSyndrome>(x, y);
    syndrome_change_list = std::make_shared<std::vector<std::shared_ptr<CodeScheme::RectSyndrome>>>();
}

RectangularSurfaceCode::RectangularSurfaceCode(int d, std::shared_ptr<ErrorModel::ErrorModelBase> _model) : RectangularSurfaceCode::RectangularSurfaceCode(d, d, _model) {}

RectangularSurfaceCode::RectangularSurfaceCode(int _x, int _y, double p) : RectangularSurfaceCode::RectangularSurfaceCode(_x, _y, std::static_pointer_cast<ErrorModel::ErrorModelBase>(std::make_shared<ErrorModel::IIDError>(p))) {}

RectangularSurfaceCode::RectangularSurfaceCode(int d, double p) : RectangularSurfaceCode::RectangularSurfaceCode(d, d, p) {}

void RectangularSurfaceCode::reset() {
    t = 0;
    scheme = std::make_shared<CodeScheme::RectScheme>(x, y);
    last_syndrome = std::make_shared<CodeScheme::RectSyndrome>(x, y);
    syndrome_change_list = std::make_shared<std::vector<std::shared_ptr<CodeScheme::RectSyndrome>>>();
}

void RectangularSurfaceCode::step(int dt) {
    for(int _ = 0; _ < dt; _++) {
        auto errors = model->generate_rectangular_error(scheme->get_shape());
        scheme->add_data_error(errors.first);
        scheme->add_syndrome_error(errors.second);
        last_error = scheme->data_error;
        syndrome_change_list->push_back(scheme->get_syndrome()^last_syndrome);
        last_syndrome = scheme->get_syndrome();
        scheme->clear_syndrome_error();
        t++;
    }
}

void RectangularSurfaceCode::manual_step(std::shared_ptr<CodeScheme::RectError> data_error, std::shared_ptr<CodeScheme::RectSyndrome> syndrome_error) {
    scheme->add_data_error(data_error);
    scheme->add_syndrome_error(syndrome_error);
    last_error = scheme->data_error;
    syndrome_change_list->push_back(scheme->get_syndrome()^last_syndrome);
    last_syndrome = scheme->get_syndrome();
    scheme->clear_syndrome_error();
    t++;
}

}