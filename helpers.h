#ifndef HELPERS_H
#define HELPERS_H
#include <QString>
#include <QFile>
#include <QTextStream>

namespace Helpers {

/**
 * @brief readFile 读取文件所有内容为QString实例
 * @param path 文件路径
 * @return 文件内容
 */
QString readFile(QString path);


/**
 * @brief The ColorStop class 线性渐变颜色转折点
 */
struct ColorStop {
    // 小数
    float percentage;
    quint8 r;
    quint8 g;
    quint8 b;
    float a;
public:
    ColorStop(float percentage, quint8 r, quint8 g,
              quint8 b, float a);

    std::vector<float> getColor()const;
};



/**
 * @brief linearGradient 线性渐变插值函数
 * @param stops 颜色转折点列表，按位置百分比排序
 * @param interpPercentage 插值位置百分比（小数）
 * @return 插值结果，为4个元素的std::vector<float>
 */
std::vector<float> linearGradient(const std::vector<ColorStop>& stops,
                                  float interpPercentage);

}

#endif // HELPERS_H
