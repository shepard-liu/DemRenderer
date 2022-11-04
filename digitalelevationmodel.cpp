#include "digitalelevationmodel.h"


quint64 DigitalElevationModel::getRows() const {
    return nRows;
}

qreal DigitalElevationModel::getLowerLeftX() const {
    return dLowerLeftX;
}

qreal DigitalElevationModel::getLowerLeftY() const {
    return dLowerLeftY;
}

qreal DigitalElevationModel::getCellSize() const {
    return dCellSize;
}

qreal DigitalElevationModel::getNoData() const {
    return dNoData;
}

const std::vector<float>& DigitalElevationModel::getData() const {
    return data;
}

quint64 DigitalElevationModel::getCols() const {
    return nCols;
}
