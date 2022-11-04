#ifndef DIGITALELEVATIONMODEL_H
#define DIGITALELEVATIONMODEL_H

#include <QFile>
#include <QString>
#include <QTextStream>
#include <vector>

/**
 * @brief The DigitalElevationModel class
 */

class DigitalElevationModel {

private:
    quint64 nCols = 0;
    quint64 nRows = 0;
    qreal dLowerLeftX = 0;
    qreal dLowerLeftY = 0;
    qreal dCellSize = 0;
    qreal dNoData = 0;
    std::vector<float> data {};

public:
    enum SourceType {
        FromText = 0x1,
        FromBinary = 0x2,
    };
    Q_DECLARE_FLAGS(SourceTypes, SourceType);

public:
    DigitalElevationModel(quint64 cols = 0, quint64 rows = 0,
                          qreal lowerLeftX = 0,
                          qreal lowerLeftY = 0,
                          qreal cellSize = 0,
                          qreal noData = 0, std::vector<float>&& data = std::vector<float>()):
        nCols(cols), nRows(rows), dLowerLeftX(lowerLeftX), dLowerLeftY(lowerLeftY),
        dCellSize(cellSize), dNoData(noData), data(data) {}



    /**
     * @brief loadDEMFromText 从ASCII编码的DEM文件读取数据
     * @param path 文件路径
     * @return DEM数据结构体
     */
    static DigitalElevationModel loadFromFile(QString path, SourceTypes type) {
        if(type.testAnyFlag(DigitalElevationModel::FromBinary)) {
            throw "Binary DEM source is not currently supported.";
        }

        QFile file(path);
        file.open(QFile::ReadOnly);
        if(!file.isOpen()) return DigitalElevationModel();
        QTextStream stream(file.readAll());

        // 读取元数据
        QString buffer{};
        int cols, rows;
        double lowerLeftX, lowerLeftY, cellSize, noData ;
        stream >> buffer >> cols
               >> buffer >> rows
               >> buffer >> lowerLeftX
               >> buffer >> lowerLeftY
               >> buffer >> cellSize
               >> buffer >> noData;

        std::vector<float> data{};
        data.reserve(cols * rows);

        // 读取格网高程数据
        for(int x = 0; x < cols; ++x) {
            for(int y = 0; y < rows; ++y) {
                float value = 0;
                stream >> value;
                data.push_back(value);
            }
        }

        return DigitalElevationModel(cols, rows, lowerLeftX, lowerLeftY, cellSize,
                                     noData,
                                     std::move(data));
    }

    /**
     * @brief 判断DEM是否无数据
     * @return true/false
     */
    bool isEmpty()const {
        return nCols * nRows == 0;
    }

    quint64 getCols() const;
    quint64 getRows() const;
    qreal getLowerLeftX() const;
    qreal getLowerLeftY() const;
    qreal getCellSize() const;
    qreal getNoData() const;
    const std::vector<float>& getData() const;
};

#endif // DIGITALELEVATIONMODEL_H
