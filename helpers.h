#ifndef HELPERS_H
#define HELPERS_H
#include <QApplication>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>

struct Helpers {

    /**
     * @brief readFile 读取文件所有内容为QString实例
     * @param path 文件路径
     * @return 文件内容
     */
    static QString readFile(QString path);


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
    static std::vector<float> linearGradient(const std::vector<ColorStop>& stops,
            float interpPercentage);

    inline static QString applicationDir{};

    inline static float Pi{static_cast<float>(std::atan2(0, -1))};

    /**
     * @brief eularMatrix 欧拉角转换到旋转矩阵(4x4)
     * @param zR 绕z轴旋转量（弧度）
     * @param yR 绕y轴旋转量（弧度）
     * @param xR 绕x轴旋转量（弧度）
     * @return
     */
    static QMatrix4x4 eulerMatrix(float zR, float yR, float xR);

    // 初始化
    inline static void init() {
#ifdef Q_OS_MACOS
        QDir dir(QApplication::applicationDirPath());
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
        Helpers::applicationDir = dir.absolutePath();
#elif Q_OS_WINDOWS
        Helpers::applicationDir = QApplication::applicationDirPath();
#endif
    }
};

#endif // HELPERS_H
