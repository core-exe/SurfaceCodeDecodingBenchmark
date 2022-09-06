#include "planar_surface_code.hpp"

namespace ErrorDynamics {

PlanarSurfaceCode::PlanarSurfaceCode(int _x, int _y, std::shared_ptr<ErrorModel::ErrorModelBase> _model) {
    t = 0;
    x = _x, y = _y;
    scheme = std::make_shared<CodeScheme::PlanarScheme>(x, y);
    model = _model;
    last_syndrome = std::make_shared<CodeScheme::PlanarSyndrome>(x, y);
    syndrome_change_list = std::make_shared<std::vector<std::shared_ptr<CodeScheme::PlanarSyndrome>>>();
}

PlanarSurfaceCode::PlanarSurfaceCode(int d, std::shared_ptr<ErrorModel::ErrorModelBase> _model) : PlanarSurfaceCode::PlanarSurfaceCode(d, d, _model) {}

PlanarSurfaceCode::PlanarSurfaceCode(int _x, int _y, double p) : PlanarSurfaceCode::PlanarSurfaceCode(_x, _y, std::static_pointer_cast<ErrorModel::ErrorModelBase>(std::make_shared<ErrorModel::IIDError>(p))) {}

PlanarSurfaceCode::PlanarSurfaceCode(int d, double p) : PlanarSurfaceCode::PlanarSurfaceCode(d, d, p) {}

void PlanarSurfaceCode::reset() {
    t = 0;
    scheme = std::make_shared<CodeScheme::PlanarScheme>(x, y);
    last_syndrome = std::make_shared<CodeScheme::PlanarSyndrome>(x, y);
    syndrome_change_list = std::make_shared<std::vector<std::shared_ptr<CodeScheme::PlanarSyndrome>>>();
}

void PlanarSurfaceCode::step(int dt) {
    for(int _ = 0; _ < dt; _++) {
        auto errors = model->generate_planar_error(scheme->get_shape());
        scheme->add_data_error(errors.first);
        scheme->add_syndrome_error(errors.second);
        last_error = scheme->data_error;
        syndrome_change_list->push_back(scheme->get_syndrome()^last_syndrome);
        last_syndrome = scheme->get_syndrome();
        scheme->clear_syndrome_error();
        t++;
    }
}

void PlanarSurfaceCode::manual_step(std::shared_ptr<CodeScheme::PlanarError> data_error, std::shared_ptr<CodeScheme::PlanarSyndrome> syndrome_error) {
    scheme->add_data_error(data_error);
    scheme->add_syndrome_error(syndrome_error);
    last_error = scheme->data_error;
    syndrome_change_list->push_back(scheme->get_syndrome()^last_syndrome);
    last_syndrome = scheme->get_syndrome();
    scheme->clear_syndrome_error();
    t++;
}

}