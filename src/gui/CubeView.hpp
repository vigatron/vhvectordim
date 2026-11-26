#ifndef CUBE_VIEW_H
#define CUBE_VIEW_H

#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QVector3D>
#include <QtGlobal>

#include <vector>

class QMouseEvent;
class QWheelEvent;

/**
 * @brief Виджет 3D-сцены.
 *
 * Показывает изометрическую ортографическую проекцию параллелепипеда,
 * собранного из кубиков. Значение 0 отображается цветом color0, 1 — color1.
 */
class CubeView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

public:
    /**
     * @brief Создаёт виджет 3D-сцены.
     * @param parent Родительский виджет.
     */
    explicit CubeView(QWidget *parent = nullptr);

    /** @brief Освобождает OpenGL-ресурсы. */
    ~CubeView() override;

    /**
     * @brief Передаёт данные и размеры параллелепипеда.
     *
     * Метод можно вызывать до создания OpenGL-контекста: буферы будут
     * загружены при инициализации.
     *
     * @param data Проверенные значения 0/1.
     * @param length Размер по оси X.
     * @param width Размер по оси Y.
     * @param height Размер по оси Z.
     */
    void setData(std::vector<quint8> data, int length, int width, int height);

    /**
     * @brief Меняет раскладку, сохраняя загруженные данные.
     * @param length Размер по оси X.
     * @param width Размер по оси Y.
     * @param height Размер по оси Z.
     */
    void setLayout(int length, int width, int height);

    /** @brief Возвращает камеру в исходный изометрический вид. */
    void resetCamera();

    /**
     * @brief Задаёт цвета элементов 0 и 1.
     * @param color0 Цвет элементов со значением 0.
     * @param color1 Цвет элементов со значением 1.
     */
    void setColors(const QColor &color0, const QColor &color1);

    /**
     * @brief Задаёт прозрачность кубиков со значением 0.
     * @param percent Прозрачность в процентах: 0 — непрозрачные, 100 — невидимые.
     */
    void setTransparency0(int percent);

    /**
     * @brief Задаёт прозрачность кубиков со значением 1.
     * @param percent Прозрачность в процентах: 0 — непрозрачные, 100 — невидимые.
     */
    void setTransparency1(int percent);

    /**
     * @brief Задаёт цвет фона 3D-сцены.
     * @param color Цвет фона.
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief Возвращает цвет элементов 0.
     * @return Текущий цвет color0.
     */
    QColor color0() const;

    /**
     * @brief Возвращает цвет элементов 1.
     * @return Текущий цвет color1.
     */
    QColor color1() const;

    /**
     * @brief Возвращает прозрачность кубиков со значением 0.
     * @return Прозрачность в процентах.
     */
    int transparency0() const;

    /**
     * @brief Возвращает прозрачность кубиков со значением 1.
     * @return Прозрачность в процентах.
     */
    int transparency1() const;

    /**
     * @brief Возвращает цвет фона 3D-сцены.
     * @return Текущий цвет фона.
     */
    QColor backgroundColor() const;

protected:
    /** @brief Инициализирует OpenGL, шейдеры и буферы. */
    void initializeGL() override;

    /** @brief Отрисовывает сцену. */
    void paintGL() override;

    /**
     * @brief Начинает вращение камеры левой кнопкой мыши.
     * @param event Событие мыши.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief Вращает камеру при перемещении мыши.
     * @param event Событие мыши.
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief Завершает вращение камеры.
     * @param event Событие мыши.
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief Сбрасывает камеру двойным кликом.
     * @param event Событие мыши.
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /**
     * @brief Изменяет масштаб колесом мыши.
     * @param event Событие колеса.
     */
    void wheelEvent(QWheelEvent *event) override;

private:
    /** @brief Данные одного инстанса кубика. */
    struct InstanceVertex {
        float x, y, z;  ///< Позиция кубика.
        float r, g, b;  ///< Цвет кубика.
        float isOne;    ///< 0 для элементов 0, 1 для элементов 1.
    };

    /** @brief Перестраивает список инстансов по текущим данным. */
    void rebuildInstances();

    /** @brief Сортирует инстансы от дальних к ближним. */
    void sortInstancesBackToFront();

    /** @brief Загружает инстансы в GPU-буфер. */
    void uploadInstances();

    /**
     * @brief Возвращает направление камеры.
     * @return Единичный вектор направления.
     */
    QVector3D cameraDirection() const;

    /**
     * @brief Возвращает радиус сцены.
     * @return Радиус описанной сферы.
     */
    float sceneRadius() const;

    std::vector<quint8> data_;
    int length_ = 0;
    int width_ = 0;
    int height_ = 0;

    QColor color0_{26, 26, 26};
    QColor color1_{230, 230, 230};
    QColor backgroundColor_{255, 255, 255};
    int transparency0_ = 50;
    int transparency1_ = 50;

    std::vector<InstanceVertex> instances_;
    bool instancesDirty_ = false;
    bool glReady_ = false;

    QOpenGLShaderProgram *program_ = nullptr;
    QOpenGLBuffer cubeVbo_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer instanceVbo_{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao_;

    float yaw_ = -45.0f;
    float pitch_ = 35.264f;
    float zoom_ = 1.0f;
    QPoint lastMousePos_;
    bool rotating_ = false;
};

#endif // CUBE_VIEW_H
