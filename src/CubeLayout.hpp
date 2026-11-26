#ifndef CUBE_LAYOUT_H
#define CUBE_LAYOUT_H

#include <cstddef>
#include <optional>
#include <vector>

/**
 * @brief Вычисляет размеры параллелепипеда L × W × H для массива элементов.
 *
 * Все стороны должны быть больше 1. Варианты упорядочены от самого
 * компактного: минимизируется площадь поверхности, затем максимальная сторона.
 */
class CubeLayout {
public:
    /** @brief Размеры параллелепипеда. */
    struct Dimensions {
        int length;  ///< Размер по оси X.
        int width;   ///< Размер по оси Y.
        int height;  ///< Размер по оси Z.
    };

    /**
     * @brief Возвращает все допустимые варианты раскладки.
     *
     * Лучший вариант находится первым.
     *
     * @param elementCount Количество элементов массива.
     * @return Список вариантов или пустой список, если раскладки нет.
     */
    static std::vector<Dimensions> variants(std::size_t elementCount);

    /**
     * @brief Возвращает самый компактный вариант раскладки.
     * @param elementCount Количество элементов массива.
     * @return Лучший вариант или std::nullopt, если раскладки нет.
     */
    static std::optional<Dimensions> compute(std::size_t elementCount);
};

#endif // CUBE_LAYOUT_H
