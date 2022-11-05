#ifndef ORBITCONTROLS_H
#define ORBITCONTROLS_H

#include <QMatrix4x4>
#include <QVector3D>
#include "helpers.h"

/**
 * @brief The OrbitControls class
 *
 * 三维场景环绕式相机控制
 */
class OrbitControls {
public:
    OrbitControls(const QVector3D &center = QVector3D(), float phi = 0, float theta = 0,
                  float radius = 0, float polarSafeZone = Helpers::Pi / 24.0f );

private:
    // 水平角
    float mPhi{};
    // 天顶角
    float mTheta{};
    // 球半径
    float mRadius{};

    // 极点安全范围
    float mPolarSafeZone{};

    // 球心位置
    QVector3D mCenter{};

public:
    /**
     * @brief setCenter 重新设置球心位置
     * @param newCenter 新的球心位置矢量
     */
    void setCenter(const QVector3D &newCenter);

    /**
     * @brief setCenter 重新设置球心位置
     * @param x 新的球心位置矢量 X分量
     * @param y 新的球心位置矢量 Y分量
     * @param z 新的球心位置矢量 Z分量
     */
    void setCenter(float x, float y, float z);

    /**
     * @brief move 在球坐标系上移动相机
     * @param deltaPhi 水平角偏移量(弧度)
     * @param deltaTheta 天顶角偏移量(弧度)
     * @param deltaRadius 球半径偏移量
     */
    void move(float deltaPhi, float deltaTheta, float deltaRadius);

    /**
     * @brief computeViewMatrix 计算视图变换矩阵
     * @return view matrix 4x4
     */
    QMatrix4x4 computeViewMatrix();

    /**
     * @brief computeRotationMatrix 计算相机姿态旋转变化矩阵
     * @return rotation matrix 4x4
     */
    QMatrix4x4 computeRotationMatrix();

    /**
     * @brief phi 获取水平角
     * @return
     */
    float phi() const;
    /**
     * @brief setPhi 设置水平角
     * @param newPhi
     */
    void setPhi(float newPhi);
    /**
     * @brief theta 获取天顶角
     * @return
     */
    float theta() const;
    /**
     * @brief setTheta 设置天顶角
     * @param newTheta
     */
    void setTheta(float newTheta);
    /**
     * @brief radius 获取球半径
     * @return
     */
    float radius() const;
    /**
     * @brief setRadius 设置球半径
     * @param newRadius
     */
    void setRadius(float newRadius);

    QString toString() const;

    /**
     * @brief center 获取相机中心点位置
     * @return
     */
    const QVector3D &center() const;
};

#endif // ORBITCONTROLS_H
