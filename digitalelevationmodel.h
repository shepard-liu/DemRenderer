#ifndef DIGITALELEVATIONMODEL_H
#define DIGITALELEVATIONMODEL_H

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QVector3D>
#include <vector>

/**
 * @brief The DigitalElevationModel class
 */

class DigitalElevationModel {

private:
    quint64 uCols = 0;
    quint64 uRows = 0;
    // 左下角像素的左下角地理坐标
    float dLowerLeftX = 0;
    float dLowerLeftY = 0;
    // DEM格网尺寸(m)
    float dCellSize = 0;
    // 无数据格网点的值
    float dNoData = 0;
    std::vector<float> data {};

public:
    enum SourceType {
        FromText = 0x1,
        FromBinary = 0x2,
    };
    Q_DECLARE_FLAGS(SourceTypes, SourceType);

public:
    DigitalElevationModel(quint64 cols = 0, quint64 rows = 0,
                          float lowerLeftX = 0,
                          float lowerLeftY = 0,
                          float cellSize = 0,
                          float noData = 0, std::vector<float>&& data = std::vector<float>()):
        uCols(cols), uRows(rows), dLowerLeftX(lowerLeftX), dLowerLeftY(lowerLeftY),
        dCellSize(cellSize), dNoData(noData), data(data) {
        // 预计算地理坐标映射常数
        affineConstantX = lowerLeftX + cellSize * (rows - 0.5);
        affineConstantY = lowerLeftY + cellSize * 0.5;
    }



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
        return uCols * uRows == 0;
    }

    quint64 getCols() const;
    quint64 getRows() const;
    float getLowerLeftX() const;
    float getLowerLeftY() const;
    float getCellSize() const;
    float getNoDataValue() const;
    const std::vector<float>& getData() const;

    /**
     * @brief getElev 获取格网点高程
     * @param row 行号
     * @param col 列号
     * @return
     */
    inline float getElev(quint64 row, quint64 col)const {
        Q_ASSERT(!isEmpty() && row < uRows && col < uCols);
        return data[row * uCols + col];
    }

    /**
     * getGeoCoord函数可缓存的常数
     */
private:
    float affineConstantX = 0.0f;
    float affineConstantY = 0.0f;

public:

    /**
     * @brief getGeoCoord 获取格网点地理坐标
     *
     * 北东高(X北，Y东，Z高)
     * @param row 行号
     * @param col 列号
     * @return
     */
    inline QVector3D getGeoCoord(quint64 row, quint64 col)const {
        Q_ASSERT(!isEmpty() && row < uRows && col < uCols);
        /**
         * 以下公式已预先推导。
         *
         * 栅格图像到地理坐标的计算通常需经过地理配准，
         * 这里简化为仿射变换关系（平移和缩放）
         */
        return QVector3D(
                   affineConstantX - dCellSize * row,
                   dCellSize * col + affineConstantY,
                   data[row * uCols + col]
               );
    }
};

#endif // DIGITALELEVATIONMODEL_H
