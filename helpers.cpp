#include <QString>
#include <QFile>
#include <QMatrix4x4>
#include "helpers.h"

QString Helpers::readFile(QString path) {
    QFile file(path);
    file.open(QFile::ReadOnly);
    if(!file.isOpen()) {
        return QString();
    }

    return QString(file.readAll());
}

Helpers::ColorStop::ColorStop(float percentage, quint8 r, quint8 g, quint8 b,
                              float a) : percentage(percentage), r(r), g(g), b(b), a(a) {}

std::vector<float> Helpers::ColorStop::getColor() const {
    return std::vector<float> {r / 255.0f, g / 255.0f, b / 255.0f, a};
}

std::vector<float> Helpers::linearGradient(const std::vector<ColorStop> &stops,
        float interpPercentage) {
    if(stops.empty())throw "Unexpected stops for linear gradient.";
    if(stops.size() == 1)return stops[0].getColor();

    std::vector<float> color{0.0f, 0.0f, 0.0f, 0.0f};
    bool foundInside = false;

    for(quint64 i = 0; i < stops.size(); ++i) {
        const ColorStop &cur = stops[i];
        if(interpPercentage > cur.percentage) continue;
        if(i == 0) {
            color = cur.getColor();
        } else {
            auto& last = stops.at(i - 1);
            auto lastColor = last.getColor();
            auto curColor = cur.getColor();

            for(quint64 i = 0 ; i < color.size(); ++i) {
                color[i] = (curColor[i] - lastColor[i]) *
                           (interpPercentage - last.percentage) /
                           (cur.percentage - last.percentage) +
                           lastColor[i];
            }
        }

        foundInside = true;
        break;

    }

    if(!foundInside) {
        return stops.back().getColor();
    }

    return color;
}

QMatrix4x4 Helpers::eulerMatrix(float zR, float yR, float xR) {
    float cZ = cos(zR), sZ = sin(zR),
          cY = cos(yR), sY = sin(yR),
          cX = cos(xR), sX = sin(xR);

    return QMatrix4x4(
               cY * cZ,     sX * sY * cZ - cX * sZ,     cX * sY * cZ + sX * sZ, 0,
               cY * sZ,     sX * sY * sZ + cX * cZ,     cX * sY * sZ - sX * cZ, 0,
               -sY,         sX * cY,                    cX * cY,                0,
               0,           0,                          0,                      1
           );
}
