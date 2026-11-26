#ifndef FILE_VALIDATOR_H
#define FILE_VALIDATOR_H

#include <QString>
#include <QtGlobal>

#include <vector>

/**
 * @brief Результат загрузки и валидации бинарного файла.
 *
 * При успехе error пуст, а bytes содержит проверенные значения 0/1.
 */
struct DataLoadResult {
    std::vector<quint8> bytes;  ///< Проверенные значения 0/1.
    QString error;              ///< Описание ошибки или пустая строка.

    /**
     * @brief Проверяет успешность загрузки.
     * @return true, если ошибки нет.
     */
    bool ok() const { return error.isEmpty(); }
};

/** @brief Загружает и проверяет бинарный файл с данными 0/1. */
class FileValidator {
public:
    /**
     * @brief Читает файл целиком и проверяет каждый байт.
     * @param fileName Путь к бинарному файлу.
     * @return Результат загрузки с данными или описанием ошибки.
     */
    static DataLoadResult load(const QString &fileName);
};

#endif // FILE_VALIDATOR_H
