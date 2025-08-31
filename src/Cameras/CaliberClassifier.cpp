#include "Cameras/CaliberClassifier.h"

int CaliberClassifier::classify(const Mensaje& msg) const {
    if (!_program) return -1;

    const auto& calibers = _program->getCalibers();
    
    for (size_t caliberIndex = 0; caliberIndex < calibers.size(); ++caliberIndex) {
        const auto& caliber = calibers[caliberIndex];
        bool matchesAllDimensions = true;

        for (size_t dimIndex = 0; dimIndex < _program->getNumDimensions(); ++dimIndex) {
            std::string dimension = _program->getDimension(dimIndex);
            double value = getValueFromMessage(msg, dimension);
            //cout << "Dimension: " << dimension << " Value: " << value << endl;
            if (!isInRange(caliber, dimIndex, value)) {
                matchesAllDimensions = false;
                break;
            }
        }

        if (matchesAllDimensions) {
            return caliberIndex;
        }
    }

    return -1;
}

bool CaliberClassifier::isInRange(const Caliber& caliber, size_t dimIndex, double value) const {
    const auto& dimensions = caliber.getDimensions();
    if (dimIndex >= dimensions.size()) return false;

    return value >= dimensions[dimIndex].getMinValue() && 
           value <= dimensions[dimIndex].getMaxValue();
}

double CaliberClassifier::getValueFromMessage(const Mensaje& msg, const std::string& dimension) const {
    if (dimension == "COLOR") {
        return msg.getColor();
    } else if (dimension == "AREA") {
        return msg.getArea();
    } else if (dimension == "DIAMETER_MIN") {
        return msg.getDiMenor();
    } else if (dimension == "DIAMETER_MAX") {
        return msg.getDiMayor();
    }
    return 0.0;
}