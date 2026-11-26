#include "CubeView.hpp"

#include <QDebug>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kCubeFill = 0.80f;
constexpr float kDefaultYaw = -45.0f;
constexpr float kDefaultPitch = 35.264f;

constexpr char kVertexShader[] = R"(
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aOffset;
layout(location = 3) in vec3 aColor;
layout(location = 4) in float aIsOne;

uniform mat4 uViewProjection;

out vec3 vNormal;
out vec3 vColor;
out float vIsOne;

void main() {
    vNormal = aNormal;
    vColor = aColor;
    vIsOne = aIsOne;
    gl_Position = uViewProjection * vec4(aPosition + aOffset, 1.0);
}
)";

constexpr char kFragmentShader[] = R"(
#version 330 core

in vec3 vNormal;
in vec3 vColor;
in float vIsOne;

uniform vec3 uLightDir;
uniform float uAlpha0;
uniform float uAlpha1;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(vNormal);
    float diffuse = max(dot(normal, normalize(uLightDir)), 0.0);
    float shade = 0.65 + 0.35 * diffuse;
    float alpha = vIsOne > 0.5 ? uAlpha1 : uAlpha0;
    fragColor = vec4(vColor * shade, alpha);
}
)";

struct CubeVertex {
    float px, py, pz;
    float nx, ny, nz;
};

void addFace(std::vector<CubeVertex> &vertices,
             const CubeVertex &v0, const CubeVertex &v1,
             const CubeVertex &v2, const CubeVertex &v3)
{
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);

    vertices.push_back(v0);
    vertices.push_back(v2);
    vertices.push_back(v3);
}

std::vector<CubeVertex> makeCubeVertices(float h) {
    const CubeVertex xp0{h, -h, -h, 1.0f, 0.0f, 0.0f};
    const CubeVertex xp1{h, h, -h, 1.0f, 0.0f, 0.0f};
    const CubeVertex xp2{h, h, h, 1.0f, 0.0f, 0.0f};
    const CubeVertex xp3{h, -h, h, 1.0f, 0.0f, 0.0f};

    const CubeVertex xn0{-h, -h, -h, -1.0f, 0.0f, 0.0f};
    const CubeVertex xn1{-h, -h, h, -1.0f, 0.0f, 0.0f};
    const CubeVertex xn2{-h, h, h, -1.0f, 0.0f, 0.0f};
    const CubeVertex xn3{-h, h, -h, -1.0f, 0.0f, 0.0f};

    const CubeVertex yp0{-h, h, -h, 0.0f, 1.0f, 0.0f};
    const CubeVertex yp1{-h, h, h, 0.0f, 1.0f, 0.0f};
    const CubeVertex yp2{h, h, h, 0.0f, 1.0f, 0.0f};
    const CubeVertex yp3{h, h, -h, 0.0f, 1.0f, 0.0f};

    const CubeVertex yn0{-h, -h, -h, 0.0f, -1.0f, 0.0f};
    const CubeVertex yn1{h, -h, -h, 0.0f, -1.0f, 0.0f};
    const CubeVertex yn2{h, -h, h, 0.0f, -1.0f, 0.0f};
    const CubeVertex yn3{-h, -h, h, 0.0f, -1.0f, 0.0f};

    const CubeVertex zp0{-h, -h, h, 0.0f, 0.0f, 1.0f};
    const CubeVertex zp1{h, -h, h, 0.0f, 0.0f, 1.0f};
    const CubeVertex zp2{h, h, h, 0.0f, 0.0f, 1.0f};
    const CubeVertex zp3{-h, h, h, 0.0f, 0.0f, 1.0f};

    const CubeVertex zn0{-h, -h, -h, 0.0f, 0.0f, -1.0f};
    const CubeVertex zn1{-h, h, -h, 0.0f, 0.0f, -1.0f};
    const CubeVertex zn2{h, h, -h, 0.0f, 0.0f, -1.0f};
    const CubeVertex zn3{h, -h, -h, 0.0f, 0.0f, -1.0f};

    std::vector<CubeVertex> vertices;
    vertices.reserve(36);

    addFace(vertices, xp0, xp1, xp2, xp3);
    addFace(vertices, xn0, xn1, xn2, xn3);
    addFace(vertices, yp0, yp1, yp2, yp3);
    addFace(vertices, yn0, yn1, yn2, yn3);
    addFace(vertices, zp0, zp1, zp2, zp3);
    addFace(vertices, zn0, zn1, zn2, zn3);

    return vertices;
}

} // namespace

CubeView::CubeView(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);
}

CubeView::~CubeView() {
    if (context() && context()->isValid()) {
        makeCurrent();

        delete program_;
        program_ = nullptr;

        cubeVbo_.destroy();
        instanceVbo_.destroy();
        vao_.destroy();

        doneCurrent();
    }
}

void CubeView::setData(std::vector<quint8> data, int length, int width, int height) {
    data_ = std::move(data);
    length_ = length;
    width_ = width;
    height_ = height;

    rebuildInstances();
    update();
}

void CubeView::setLayout(int length, int width, int height) {
    length_ = length;
    width_ = width;
    height_ = height;

    rebuildInstances();
    update();
}

void CubeView::resetCamera() {
    yaw_ = kDefaultYaw;
    pitch_ = kDefaultPitch;
    zoom_ = 1.0f;
    instancesDirty_ = true;
    update();
}

void CubeView::setColors(const QColor &color0, const QColor &color1) {
    if (!color0.isValid() || !color1.isValid()) {
        qWarning() << "Ошибка: недопустимый цвет элементов";
        return;
    }

    color0_ = color0;
    color1_ = color1;
    rebuildInstances();
    update();
}

void CubeView::setTransparency0(int percent) {
    transparency0_ = std::clamp(percent, 0, 100);
    update();
}

void CubeView::setTransparency1(int percent) {
    transparency1_ = std::clamp(percent, 0, 100);
    update();
}

void CubeView::setBackgroundColor(const QColor &color) {
    if (!color.isValid()) {
        qWarning() << "Ошибка: недопустимый цвет фона";
        return;
    }

    backgroundColor_ = color;
    update();
}

QColor CubeView::color0() const {
    return color0_;
}

QColor CubeView::color1() const {
    return color1_;
}

int CubeView::transparency0() const {
    return transparency0_;
}

int CubeView::transparency1() const {
    return transparency1_;
}

QColor CubeView::backgroundColor() const {
    return backgroundColor_;
}

void CubeView::initializeGL() {
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_MULTISAMPLE);

    qInfo() << "OpenGL:" << reinterpret_cast<const char *>(glGetString(GL_VERSION));

    program_ = new QOpenGLShaderProgram();
    if (!program_->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader)) {
        qWarning().noquote() << "Ошибка компиляции вершинного шейдера:\n" << program_->log();
        return;
    }
    if (!program_->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader)) {
        qWarning().noquote() << "Ошибка компиляции фрагментного шейдера:\n" << program_->log();
        return;
    }
    if (!program_->link()) {
        qWarning().noquote() << "Ошибка линковки шейдерной программы:\n" << program_->log();
        return;
    }

    const std::vector<CubeVertex> cubeVertices = makeCubeVertices(0.5f * kCubeFill);

    vao_.create();
    vao_.bind();

    cubeVbo_.create();
    cubeVbo_.bind();
    cubeVbo_.allocate(cubeVertices.data(),
                      static_cast<int>(cubeVertices.size() * sizeof(CubeVertex)));

    program_->enableAttributeArray(0);
    program_->setAttributeBuffer(0, GL_FLOAT, offsetof(CubeVertex, px),
                                 3, sizeof(CubeVertex));
    program_->enableAttributeArray(1);
    program_->setAttributeBuffer(1, GL_FLOAT, offsetof(CubeVertex, nx),
                                 3, sizeof(CubeVertex));

    instanceVbo_.create();
    instanceVbo_.bind();

    program_->enableAttributeArray(2);
    program_->setAttributeBuffer(2, GL_FLOAT, offsetof(InstanceVertex, x),
                                 3, sizeof(InstanceVertex));
    glVertexAttribDivisor(2, 1);

    program_->enableAttributeArray(3);
    program_->setAttributeBuffer(3, GL_FLOAT, offsetof(InstanceVertex, r),
                                 3, sizeof(InstanceVertex));
    glVertexAttribDivisor(3, 1);

    program_->enableAttributeArray(4);
    program_->setAttributeBuffer(4, GL_FLOAT, offsetof(InstanceVertex, isOne),
                                 1, sizeof(InstanceVertex));
    glVertexAttribDivisor(4, 1);

    vao_.release();
    cubeVbo_.release();
    instanceVbo_.release();

    glReady_ = true;
}

void CubeView::paintGL() {
    glClearColor(static_cast<float>(backgroundColor_.redF()),
                 static_cast<float>(backgroundColor_.greenF()),
                 static_cast<float>(backgroundColor_.blueF()),
                 1.0f);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);

    if (!glReady_ || !program_) {
        return;
    }

    if (instancesDirty_) {
        sortInstancesBackToFront();
        uploadInstances();
    }

    if (instances_.empty()) {
        return;
    }

    const float aspect = height() > 0
                             ? static_cast<float>(width()) / static_cast<float>(height())
                             : 1.0f;
    const float radius = sceneRadius();
    const float halfHeight = radius * 1.10f / zoom_;

    QMatrix4x4 projection;
    if (aspect >= 1.0f) {
        projection.ortho(-halfHeight * aspect, halfHeight * aspect,
                         -halfHeight, halfHeight, 0.1f, radius * 12.0f);
    } else {
        projection.ortho(-halfHeight, halfHeight,
                         -halfHeight / aspect, halfHeight / aspect,
                         0.1f, radius * 12.0f);
    }

    const QVector3D eye = cameraDirection() * (radius * 4.0f);
    QMatrix4x4 view;
    view.lookAt(eye, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f));

    const QMatrix4x4 viewProjection = projection * view;

    program_->bind();
    program_->setUniformValue("uViewProjection", viewProjection);
    program_->setUniformValue("uLightDir",
                              QVector3D(0.35f, -0.55f, 0.76f).normalized());
    program_->setUniformValue(
        "uAlpha0", 1.0f - static_cast<float>(transparency0_) / 100.0f);
    program_->setUniformValue(
        "uAlpha1", 1.0f - static_cast<float>(transparency1_) / 100.0f);

    QOpenGLVertexArrayObject::Binder binder(&vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36,
                          static_cast<GLsizei>(instances_.size()));

    program_->release();
}

void CubeView::rebuildInstances() {
    instances_.clear();
    instancesDirty_ = true;

    if (length_ <= 1 || width_ <= 1 || height_ <= 1) {
        qWarning() << "Ошибка: размеры параллелепипеда должны быть больше 1";
        return;
    }

    const std::size_t l = static_cast<std::size_t>(length_);
    const std::size_t w = static_cast<std::size_t>(width_);
    const std::size_t h = static_cast<std::size_t>(height_);

    if (l > data_.size() / w || l * w > data_.size() / h) {
        qWarning() << "Ошибка: размеры не соответствуют размеру данных";
        return;
    }

    const std::size_t expected = l * w * h;
    if (expected != data_.size()) {
        qWarning() << "Ошибка: размеры" << length_ << "x" << width_ << "x" << height_
                   << "не соответствуют количеству элементов" << data_.size();
        return;
    }

    instances_.reserve(expected);

    const float centerX = (static_cast<float>(length_) - 1.0f) * 0.5f;
    const float centerY = (static_cast<float>(width_) - 1.0f) * 0.5f;
    const float centerZ = (static_cast<float>(height_) - 1.0f) * 0.5f;

    for (int z = 0; z < height_; ++z) {
        for (int y = 0; y < width_; ++y) {
            for (int x = 0; x < length_; ++x) {
                const std::size_t index =
                    static_cast<std::size_t>(x) +
                    l * (static_cast<std::size_t>(y) +
                         w * static_cast<std::size_t>(z));

                const bool isOne = data_[index] != 0;
                const QColor &color = isOne ? color1_ : color0_;
                instances_.push_back(InstanceVertex{
                    static_cast<float>(x) - centerX,
                    static_cast<float>(y) - centerY,
                    static_cast<float>(z) - centerZ,
                    static_cast<float>(color.redF()),
                    static_cast<float>(color.greenF()),
                    static_cast<float>(color.blueF()),
                    isOne ? 1.0f : 0.0f
                });
            }
        }
    }

    instancesDirty_ = true;
}

void CubeView::sortInstancesBackToFront() {
    if (instances_.empty()) {
        return;
    }

    const QVector3D direction = cameraDirection();
    std::sort(instances_.begin(), instances_.end(),
              [&direction](const InstanceVertex &lhs, const InstanceVertex &rhs) {
                  const float lhsDepth =
                      lhs.x * direction.x() +
                      lhs.y * direction.y() +
                      lhs.z * direction.z();
                  const float rhsDepth =
                      rhs.x * direction.x() +
                      rhs.y * direction.y() +
                      rhs.z * direction.z();
                  return lhsDepth < rhsDepth;
              });
}

void CubeView::uploadInstances() {
    if (!glReady_ || !instanceVbo_.isCreated()) {
        return;
    }

    const std::size_t byteCount = instances_.size() * sizeof(InstanceVertex);
    if (byteCount > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        qWarning() << "Ошибка: слишком много кубиков для одного буфера";
        return;
    }

    instanceVbo_.bind();
    instanceVbo_.allocate(
        instances_.empty() ? nullptr : instances_.data(),
        static_cast<int>(byteCount));
    instanceVbo_.release();

    instancesDirty_ = false;
}

QVector3D CubeView::cameraDirection() const {
    const float yaw = yaw_ * kPi / 180.0f;
    const float pitch = pitch_ * kPi / 180.0f;
    const float cosPitch = std::cos(pitch);

    return QVector3D(cosPitch * std::cos(yaw),
                     cosPitch * std::sin(yaw),
                     std::sin(pitch));
}

float CubeView::sceneRadius() const {
    const float l = static_cast<float>(length_);
    const float w = static_cast<float>(width_);
    const float h = static_cast<float>(height_);
    return 0.5f * std::sqrt(l * l + w * w + h * h);
}

void CubeView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        rotating_ = true;
        lastMousePos_ = event->position().toPoint();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QOpenGLWidget::mousePressEvent(event);
}

void CubeView::mouseMoveEvent(QMouseEvent *event) {
    if (rotating_) {
        const QPoint pos = event->position().toPoint();
        const QPoint delta = pos - lastMousePos_;
        lastMousePos_ = pos;

        yaw_ += static_cast<float>(delta.x()) * 0.4f;
        pitch_ = std::clamp(pitch_ - static_cast<float>(delta.y()) * 0.4f,
                           -85.0f, 85.0f);
        instancesDirty_ = true;
        update();
        event->accept();
        return;
    }

    QOpenGLWidget::mouseMoveEvent(event);
}

void CubeView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && rotating_) {
        rotating_ = false;
        unsetCursor();
        event->accept();
        return;
    }

    QOpenGLWidget::mouseReleaseEvent(event);
}

void CubeView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        resetCamera();
        event->accept();
        return;
    }

    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void CubeView::wheelEvent(QWheelEvent *event) {
    const float steps = static_cast<float>(event->angleDelta().y()) / 120.0f;
    zoom_ = std::clamp(zoom_ * std::pow(1.15f, steps), 0.2f, 10.0f);
    update();
    event->accept();
}
