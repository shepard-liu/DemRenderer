#include "orbitcontrols.h"

OrbitControls::OrbitControls(const QVector3D &center, float phi, float theta, float radius,
                             float polarSafeZone) :
    mPhi(phi),
    mTheta(theta),
    mRadius(radius),
    mCenter(center),
    mPolarSafeZone(polarSafeZone)
{}

const QVector3D &OrbitControls::center() const {
    return mCenter;
}

float OrbitControls::phi() const {
    return mPhi;
}

void OrbitControls::setPhi(float newPhi) {
    mPhi = fmod(newPhi, (2.0f * Helpers::Pi));
}

float OrbitControls::theta() const {
    return mTheta;
}

void OrbitControls::setTheta(float newTheta) {
    if(newTheta > Helpers::Pi - mPolarSafeZone)
        mTheta = Helpers::Pi - mPolarSafeZone;
    else if(newTheta < mPolarSafeZone)
        mTheta = mPolarSafeZone;
    else
        mTheta = newTheta;
}

float OrbitControls::radius() const {
    return mRadius;
}

void OrbitControls::setRadius(float newRadius) {
    mRadius = newRadius;
    if(mRadius < 0)mRadius = 0;
}

QString OrbitControls::toString() const {
    QString output{ "OrbitControls: "};
    QTextStream stream(&output);
    stream << "Phi = " << mPhi << ", Theta = " << mTheta << ", Radius = " <<
           mRadius;
    return output;
}

void OrbitControls::setCenter(const QVector3D &newCenter) {
    mCenter = newCenter;
}

void OrbitControls::setCenter(float x, float y, float z) {
    mCenter = QVector3D(x, y, z);
}

void OrbitControls::move(float deltaPhi, float deltaTheta, float deltaRadius) {
    mPhi += deltaPhi;
    mTheta += deltaTheta;
    mRadius += deltaRadius;
}

QMatrix4x4 OrbitControls::computeViewMatrix() {
    QMatrix4x4 matrix{};

    float sinT = sin(mTheta), cosT = cos(mTheta),
          sinP = sin(mPhi), cosP = cos(mPhi);

    // 平移到球面点
    matrix.translate(
        mCenter.x() + mRadius * sinT * cosP,
        mCenter.y() + mRadius * sinT * sinP,
        mCenter.z() + mRadius * cosT
    );

    // 旋转确定相机姿态
    matrix *= QMatrix4x4(
                  -sinP,    -cosP * cosT,   cosP * sinT,    0,
                  cosP,     -cosT * sinP,   sinP * sinT,    0,
                  0,        sinT,           cosT,           0,
                  0, 0, 0, 1
              );

    return matrix.inverted();
}

QMatrix4x4 OrbitControls::computeRotationMatrix() {
    float sinT = sin(mTheta), cosT = cos(mTheta),
          sinP = sin(mPhi), cosP = cos(mPhi);

    /**
     * 旋转确定相机姿态
     *
     * 为两次旋转的化简形式
     * 等同于乘以 Helpers::eulerMatrix(mPhi, mTheta - Helpers::Pi / 2.0f, 0)
     * 然后再乘以 Helpers::eulerMatrix(Helpers::Pi / 2.0f, 0, Helpers::Pi / 2.0f)
     */

    return QMatrix4x4(
               -sinP,    -cosP * cosT,   cosP * sinT,    0,
               cosP,     -cosT * sinP,   sinP * sinT,    0,
               0,        sinT,           cosT,           0,
               0, 0, 0, 1
           );
}
