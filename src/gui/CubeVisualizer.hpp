#ifndef CUBE_VISUALIZER_H
#define CUBE_VISUALIZER_H

#include <QMainWindow>
#include <QString>
#include <QtGlobal>

#include <cstddef>
#include <vector>

#include "CubeLayout.hpp"

class CubeView;
class QFrame;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;
class QResizeEvent;
class QSlider;

/**
 * @brief Главное окно приложения.
 *
 * Содержит 3D-сцену, панель управления, меню и строку состояния.
 */
class CubeVisualizer : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Создаёт главное окно и загружает переданные данные.
     * @param fileName Имя файла с данными.
     * @param data Проверенные значения 0/1.
     * @param variants Допустимые варианты раскладки.
     * @param parent Родительский виджет.
     */
    explicit CubeVisualizer(const QString &fileName, std::vector<quint8> data,
                            std::vector<CubeLayout::Dimensions> variants,
                            QWidget *parent = nullptr);

protected:
    /**
     * @brief Поддерживает ширину панели управления на уровне 20%.
     * @param event Событие изменения размера окна.
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /** @brief Открывает диалог выбора и загружает новый файл. */
    void openFile();

    /** @brief Сбрасывает камеру в исходное изометрическое положение. */
    void resetCamera();

    /**
     * @brief Применяет выбранный вариант раскладки.
     * @param row Строка списка вариантов.
     */
    void onLayoutSelected(int row);

    /** @brief Открывает диалог выбора цвета элементов 0. */
    void chooseColor0();

    /** @brief Открывает диалог выбора цвета элементов 1. */
    void chooseColor1();

    /** @brief Открывает диалог выбора цвета фона 3D-сцены. */
    void chooseColorB();

    /**
     * @brief Обновляет прозрачность элементов 0.
     * @param value Прозрачность в процентах от 0 до 100.
     */
    void onTransparency0Changed(int value);

    /**
     * @brief Обновляет прозрачность элементов 1.
     * @param value Прозрачность в процентах от 0 до 100.
     */
    void onTransparency1Changed(int value);

    /** @brief Сохраняет изображение 3D-сцены в JPEG-файл. */
    void saveScreenshot();

private:
    /** @brief Создаёт все виджеты и элементы управления. */
    void CreateObjects();

    /** @brief Создаёт компоновку виджетов. */
    void CreateLayout();

    /** @brief Создаёт меню приложения. */
    void createMenus();

    /** @brief Заполняет список вариантов раскладки. */
    void populateLayoutList();

    /**
     * @brief Применяет новые данные и варианты раскладки.
     * @param fileName Имя файла.
     * @param data Проверенные значения 0/1.
     * @param variants Варианты раскладки.
     */
    void applyData(const QString &fileName, std::vector<quint8> data,
                   std::vector<CubeLayout::Dimensions> variants);

    /** @brief Обновляет цветные образцы на кнопках. */
    void updateColorButtons();

    /** @brief Обновляет строку состояния. */
    void updateStatus();

    CubeView *cubeView_ = nullptr;
    QWidget *centralWidget_ = nullptr;
    QFrame *panel_ = nullptr;
    QLabel *titleLabel_ = nullptr;
    QListWidget *layoutList_ = nullptr;
    QGroupBox *colorGroup_ = nullptr;
    QGroupBox *transparencyGroup_ = nullptr;
    QGroupBox *miscGroup_ = nullptr;
    QPushButton *color0Button_ = nullptr;
    QPushButton *color1Button_ = nullptr;
    QPushButton *backgroundButton_ = nullptr;
    QLabel *color0Label_ = nullptr;
    QLabel *color1Label_ = nullptr;
    QSlider *transparency0Slider_ = nullptr;
    QSlider *transparency1Slider_ = nullptr;
    QLabel *transparency0ValueLabel_ = nullptr;
    QLabel *transparency1ValueLabel_ = nullptr;
    QPushButton *saveButton_ = nullptr;
    QLabel *hintLabel_ = nullptr;
    std::vector<CubeLayout::Dimensions> variants_;
    QString fileName_;
    int length_ = 0;
    int width_ = 0;
    int height_ = 0;
    std::size_t zeros_ = 0;
    std::size_t ones_ = 0;
};

#endif // CUBE_VISUALIZER_H
