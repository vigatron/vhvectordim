#include "FileValidator.hpp"

#include <QByteArray>
#include <QFile>
#include <QFileDevice>

#include <cstddef>

DataLoadResult FileValidator::load(const QString &fileName) {
    DataLoadResult result;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Не удалось открыть файл \"%1\": %2")
                           .arg(fileName, file.errorString());
        return result;
    }

    const QByteArray raw = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        result.error = QStringLiteral("Ошибка чтения файла \"%1\": %2")
                           .arg(fileName, file.errorString());
        return result;
    }

    if (raw.isEmpty()) {
        result.error = QStringLiteral("Файл пуст");
        return result;
    }

    result.bytes.reserve(static_cast<std::size_t>(raw.size()));

    for (qsizetype i = 0; i < raw.size(); ++i) {
        const quint8 byte = static_cast<quint8>(raw.at(i));
        if (byte != 0 && byte != 1) {
            result.error = QStringLiteral("Байт %1 в позиции %2 не равен 0 или 1")
                               .arg(static_cast<int>(byte))
                               .arg(static_cast<qlonglong>(i));
            result.bytes.clear();
            return result;
        }
        result.bytes.push_back(byte);
    }

    return result;
}
