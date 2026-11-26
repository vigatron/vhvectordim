#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QSurfaceFormat>

#include <utility>
#include <vector>

#include "CubeLayout.hpp"
#include "CubeVisualizer.hpp"
#include "FileValidator.hpp"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    QApplication::setApplicationName("vhappvectordim");
    QApplication::setApplicationVersion(QStringLiteral(VHAPPVECTORDIM_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription("Программа визуализации вектора данных в 3D");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(
        "file",
        "Бинарный файл с данными (0 и 1)",
        "file"
    );

    parser.process(app);

    if (parser.positionalArguments().size() != 1) {
        qWarning() << "Ошибка: не указан файл для визуализации";
        return 1;
    }

    const QString fileName = parser.positionalArguments().constFirst();

    qDebug() << "Загрузка файла:" << fileName;

    DataLoadResult loadResult = FileValidator::load(fileName);
    if (!loadResult.ok()) {
        qWarning().noquote() << "Ошибка:" << loadResult.error;
        return 1;
    }

    const std::size_t elementCount = loadResult.bytes.size();
    qDebug() << "Файл валиден, размер массива:" << elementCount;

    // Все варианты раскладки; первый — самый компактный
    std::vector<CubeLayout::Dimensions> variants =
        CubeLayout::variants(elementCount);

    if (variants.empty()) {
        qWarning() << "Ошибка: не удалось разложить данные в параллелепипед"
                   << "(все стороны должны быть больше 1)";
        return 1;
    }

    const CubeLayout::Dimensions &best = variants.front();
    qDebug() << "Раскладка:" << best.length << "x"
             << best.width << "x" << best.height
             << "; вариантов:" << variants.size();

    // Запуск визуализатора; данные передаются без повторного чтения файла
    CubeVisualizer visualizer(fileName, std::move(loadResult.bytes),
                              std::move(variants));
    visualizer.show();
    return app.exec();
}
