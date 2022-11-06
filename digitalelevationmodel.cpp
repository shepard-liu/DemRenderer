#include "digitalelevationmodel.h"


quint64 DigitalElevationModel::getRows() const {
    return uRows;
}

float DigitalElevationModel::getLowerLeftX() const {
    return dLowerLeftX;
}

float DigitalElevationModel::getLowerLeftY() const {
    return dLowerLeftY;
}

float DigitalElevationModel::getCellSize() const {
    return dCellSize;
}

float DigitalElevationModel::getNoDataValue() const {
    return dNoData;
}

const std::vector<float>& DigitalElevationModel::getData() const {
    return data;
}

quint64 DigitalElevationModel::getCols() const {
    return uCols;
}
