#include "CubeVisualizer.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QColor>
#include <QColorDialog>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QSlider>
#include <QStatusBar>
#include <QVBoxLayout>

#include <algorithm>
#include <cstddef>
#include <utility>

#include "CubeView.hpp"
#include "FileValidator.hpp"

namespace {

void setColorButton(QPushButton *button, const QColor &color) {
    if (!button) {
        return;
    }

    QPixmap pixmap(18, 18);
    pixmap.fill(color);

    QPainter painter(&pixmap);
    painter.setPen(QColor(90, 90, 90));
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);
    painter.end();

    button->setIcon(QIcon(pixmap));
    button->setIconSize(QSize(18, 18));
    button->setToolTip(color.name(QColor::HexRgb));
}

} // namespace

CubeVisualizer::CubeVisualizer(const QString &fileName, std::vector<quint8> data,
                               std::vector<CubeLayout::Dimensions> variants,
                               QWidget *parent)
    : QMainWindow(parent)
{
    CreateObjects();
    CreateLayout();
    createMenus();

    applyData(fileName, std::move(data), std::move(variants));
    setMinimumSize(800, 600);
    resize(1000, 800);
}

void CubeVisualizer::CreateObjects() {
    cubeView_ = new CubeView(this);

    centralWidget_ = new QWidget(this);

    panel_ = new QFrame(centralWidget_);
    panel_->setFrameShape(QFrame::StyledPanel);
    panel_->setMinimumWidth(160);
    panel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    titleLabel_ = new QLabel(tr("Варианты раскладки"), panel_);
    QFont titleFont = titleLabel_->font();
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);

    layoutList_ = new QListWidget(panel_);
    layoutList_->setObjectName(QStringLiteral("wdgVariants"));
    layoutList_->setAlternatingRowColors(true);
    layoutList_->setUniformItemSizes(true);
    layoutList_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(layoutList_, &QListWidget::currentRowChanged,
            this, &CubeVisualizer::onLayoutSelected);

    colorGroup_ = new QGroupBox(tr("Цвета"), panel_);
    colorGroup_->setObjectName(QStringLiteral("wdgColor"));

    color0Button_ = new QPushButton(tr("color0"), colorGroup_);
    color0Button_->setObjectName(QStringLiteral("wdgColor0"));
    color1Button_ = new QPushButton(tr("color1"), colorGroup_);
    color1Button_->setObjectName(QStringLiteral("wdgColor1"));

    backgroundButton_ = new QPushButton(tr("colorb"), colorGroup_);
    backgroundButton_->setObjectName(QStringLiteral("wdgColorB"));

    connect(color0Button_, &QPushButton::clicked,
            this, &CubeVisualizer::chooseColor0);
    connect(color1Button_, &QPushButton::clicked,
            this, &CubeVisualizer::chooseColor1);
    connect(backgroundButton_, &QPushButton::clicked,
            this, &CubeVisualizer::chooseColorB);

    transparencyGroup_ = new QGroupBox(tr("Прозрачность"), panel_);
    transparencyGroup_->setObjectName(QStringLiteral("wdgTrans"));

    color0Label_ = new QLabel(tr("color0"), transparencyGroup_);
    color1Label_ = new QLabel(tr("color1"), transparencyGroup_);

    transparency0Slider_ = new QSlider(Qt::Horizontal, transparencyGroup_);
    transparency0Slider_->setObjectName(QStringLiteral("wdgTrans0"));
    transparency0Slider_->setRange(0, 100);
    transparency0Slider_->setValue(cubeView_->transparency0());
    transparency0Slider_->setSingleStep(1);
    transparency0Slider_->setPageStep(10);

    transparency1Slider_ = new QSlider(Qt::Horizontal, transparencyGroup_);
    transparency1Slider_->setObjectName(QStringLiteral("wdgTrans1"));
    transparency1Slider_->setRange(0, 100);
    transparency1Slider_->setValue(cubeView_->transparency1());
    transparency1Slider_->setSingleStep(1);
    transparency1Slider_->setPageStep(10);

    transparency0ValueLabel_ = new QLabel(
        QStringLiteral("%1%").arg(cubeView_->transparency0()),
        transparencyGroup_);
    transparency0ValueLabel_->setObjectName(QStringLiteral("wdgTrans0Value"));
    transparency0ValueLabel_->setAlignment(Qt::AlignCenter);

    transparency1ValueLabel_ = new QLabel(
        QStringLiteral("%1%").arg(cubeView_->transparency1()),
        transparencyGroup_);
    transparency1ValueLabel_->setObjectName(QStringLiteral("wdgTrans1Value"));
    transparency1ValueLabel_->setAlignment(Qt::AlignCenter);

    connect(transparency0Slider_, &QSlider::valueChanged,
            this, &CubeVisualizer::onTransparency0Changed);
    connect(transparency1Slider_, &QSlider::valueChanged,
            this, &CubeVisualizer::onTransparency1Changed);

    miscGroup_ = new QGroupBox(tr("Прочее"), panel_);
    miscGroup_->setObjectName(QStringLiteral("wdgMisc"));

    saveButton_ = new QPushButton(tr("Save Screen"), miscGroup_);
    saveButton_->setObjectName(QStringLiteral("btnSaveScreen"));
    connect(saveButton_, &QPushButton::clicked,
            this, &CubeVisualizer::saveScreenshot);

    hintLabel_ = new QLabel(
        tr("ЛКМ — вращение · колесо — зум · R или двойной клик — сброс"), this);

    updateColorButtons();
}

void CubeVisualizer::CreateLayout() {
    auto *centralLayout = new QHBoxLayout(centralWidget_);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    auto *panelLayout = new QVBoxLayout(panel_);
    panelLayout->setContentsMargins(8, 8, 8, 8);
    panelLayout->setSpacing(6);

    auto *colorLayout = new QGridLayout(colorGroup_);
    colorLayout->setContentsMargins(6, 6, 6, 6);
    colorLayout->setSpacing(4);
    colorLayout->addWidget(color0Button_, 0, 0);
    colorLayout->addWidget(color1Button_, 1, 0);
    colorLayout->addWidget(backgroundButton_, 2, 0);

    auto *transparencyLayout = new QGridLayout(transparencyGroup_);
    transparencyLayout->setContentsMargins(6, 6, 6, 6);
    transparencyLayout->setSpacing(4);
    transparencyLayout->setColumnStretch(1, 1);
    transparencyLayout->addWidget(color0Label_, 0, 0);
    transparencyLayout->addWidget(transparency0Slider_, 0, 1);
    transparencyLayout->addWidget(transparency0ValueLabel_, 0, 2);
    transparencyLayout->addWidget(color1Label_, 1, 0);
    transparencyLayout->addWidget(transparency1Slider_, 1, 1);
    transparencyLayout->addWidget(transparency1ValueLabel_, 1, 2);

    auto *miscLayout = new QVBoxLayout(miscGroup_);
    miscLayout->setContentsMargins(6, 6, 6, 6);
    miscLayout->setSpacing(4);
    miscLayout->addWidget(saveButton_);

    panelLayout->addWidget(titleLabel_);
    panelLayout->addWidget(layoutList_, 1);
    panelLayout->addWidget(colorGroup_);
    panelLayout->addWidget(transparencyGroup_);
    panelLayout->addWidget(miscGroup_);

    // 3D-сцена занимает 80% ширины, панель раскладок — 20%.
    centralLayout->addWidget(cubeView_, 4);
    centralLayout->addWidget(panel_, 1);

    setCentralWidget(centralWidget_);
    statusBar()->addPermanentWidget(hintLabel_);
}

void CubeVisualizer::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if (!panel_ || !centralWidget_) {
        return;
    }

    const int panelWidth = qMax(160, (centralWidget_->width() * 20 + 50) / 100);
    panel_->setFixedWidth(panelWidth);
}

void CubeVisualizer::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&Файл"));

    auto *openAction = fileMenu->addAction(tr("&Открыть..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &CubeVisualizer::openFile);

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction(tr("В&ыход"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *viewMenu = menuBar()->addMenu(tr("&Вид"));

    auto *resetAction = viewMenu->addAction(tr("&Сбросить камеру"));
    resetAction->setShortcut(QKeySequence(Qt::Key_R));
    connect(resetAction, &QAction::triggered, this, &CubeVisualizer::resetCamera);
}

void CubeVisualizer::openFile() {
    const QString initialDir = fileName_.isEmpty()
                                   ? QDir::homePath()
                                   : QFileInfo(fileName_).absolutePath();

    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Открыть бинарный файл"),
        initialDir,
        tr("Бинарные файлы (*.bin);;Все файлы (*)"));

    if (path.isEmpty()) {
        return;
    }

    DataLoadResult loaded = FileValidator::load(path);
    if (!loaded.ok()) {
        QMessageBox::warning(this, tr("Ошибка загрузки"), loaded.error);
        return;
    }

    std::vector<CubeLayout::Dimensions> variants =
        CubeLayout::variants(loaded.bytes.size());

    if (variants.empty()) {
        QMessageBox::warning(
            this,
            tr("Ошибка раскладки"),
            tr("Не удалось разложить %1 элементов в параллелепипед.\n"
               "Все стороны должны быть больше 1.")
                .arg(static_cast<qulonglong>(loaded.bytes.size())));
        return;
    }

    applyData(path, std::move(loaded.bytes), std::move(variants));
}

void CubeVisualizer::resetCamera() {
    cubeView_->resetCamera();
}

void CubeVisualizer::onLayoutSelected(int row) {
    if (row < 0 || row >= static_cast<int>(variants_.size())) {
        return;
    }

    const CubeLayout::Dimensions &dimensions =
        variants_[static_cast<std::size_t>(row)];

    length_ = dimensions.length;
    width_ = dimensions.width;
    height_ = dimensions.height;

    cubeView_->setLayout(length_, width_, height_);
    updateStatus();
}

void CubeVisualizer::chooseColor0() {
    const QColor color = QColorDialog::getColor(
        cubeView_->color0(), this, tr("Цвет элементов 0"));
    if (!color.isValid()) {
        return;
    }

    cubeView_->setColors(color, cubeView_->color1());
    updateColorButtons();
}

void CubeVisualizer::chooseColor1() {
    const QColor color = QColorDialog::getColor(
        cubeView_->color1(), this, tr("Цвет элементов 1"));
    if (!color.isValid()) {
        return;
    }

    cubeView_->setColors(cubeView_->color0(), color);
    updateColorButtons();
}

void CubeVisualizer::chooseColorB() {
    const QColor color = QColorDialog::getColor(
        cubeView_->backgroundColor(), this, tr("Цвет фона 3D"));
    if (!color.isValid()) {
        return;
    }

    cubeView_->setBackgroundColor(color);
    updateColorButtons();
}

void CubeVisualizer::onTransparency0Changed(int value) {
    if (transparency0ValueLabel_) {
        transparency0ValueLabel_->setText(QStringLiteral("%1%").arg(value));
    }

    cubeView_->setTransparency0(value);
}

void CubeVisualizer::onTransparency1Changed(int value) {
    if (transparency1ValueLabel_) {
        transparency1ValueLabel_->setText(QStringLiteral("%1%").arg(value));
    }

    cubeView_->setTransparency1(value);
}

void CubeVisualizer::saveScreenshot() {
    const QImage image = cubeView_->grabFramebuffer();
    if (image.isNull()) {
        QMessageBox::warning(
            this, tr("Ошибка снимка"),
            tr("Не удалось получить изображение 3D-сцены."));
        return;
    }

    const QString baseName = fileName_.isEmpty()
                                 ? QStringLiteral("vhappvectordim")
                                 : QFileInfo(fileName_).completeBaseName();
    const QString timestamp = QDateTime::currentDateTime().toString(
        QStringLiteral("yyyyMMdd_HHmmss"));
    const QString initialDir = fileName_.isEmpty()
                                   ? QDir::currentPath()
                                   : QFileInfo(fileName_).absolutePath();
    const QString defaultPath = QDir(initialDir).filePath(
        QStringLiteral("%1_%2.jpg").arg(baseName, timestamp));

    QString path = QFileDialog::getSaveFileName(
        this, tr("Сохранить снимок 3D"), defaultPath,
        tr("JPEG (*.jpg *.jpeg)"));
    if (path.isEmpty()) {
        return;
    }

    if (!path.endsWith(QStringLiteral(".jpg"), Qt::CaseInsensitive) &&
        !path.endsWith(QStringLiteral(".jpeg"), Qt::CaseInsensitive)) {
        path += QStringLiteral(".jpg");
    }

    if (!image.save(path, "JPG", 95)) {
        QMessageBox::warning(
            this, tr("Ошибка сохранения"),
            tr("Не удалось сохранить файл \"%1\".").arg(path));
    }
}

void CubeVisualizer::updateColorButtons() {
    setColorButton(color0Button_, cubeView_->color0());
    setColorButton(color1Button_, cubeView_->color1());
    setColorButton(backgroundButton_, cubeView_->backgroundColor());
}

void CubeVisualizer::applyData(const QString &fileName, std::vector<quint8> data,
                               std::vector<CubeLayout::Dimensions> variants) {
    if (variants.empty()) {
        return;
    }

    fileName_ = fileName;
    variants_ = std::move(variants);

    const CubeLayout::Dimensions &best = variants_.front();
    length_ = best.length;
    width_ = best.width;
    height_ = best.height;

    ones_ = static_cast<std::size_t>(
        std::count(data.begin(), data.end(), quint8{1}));
    zeros_ = data.size() - ones_;

    cubeView_->setData(std::move(data), length_, width_, height_);
    cubeView_->resetCamera();
    populateLayoutList();

    setWindowTitle(QStringLiteral("VHAppVectorDim — %1")
                       .arg(QFileInfo(fileName_).fileName()));
    updateStatus();
}

void CubeVisualizer::populateLayoutList() {
    layoutList_->blockSignals(true);
    layoutList_->clear();

    for (const CubeLayout::Dimensions &dimensions : variants_) {
        layoutList_->addItem(QStringLiteral("%1x%2x%3")
                                 .arg(dimensions.length)
                                 .arg(dimensions.width)
                                 .arg(dimensions.height));
    }

    if (layoutList_->count() > 0) {
        layoutList_->setCurrentRow(0);
    }

    layoutList_->blockSignals(false);
}

void CubeVisualizer::updateStatus() {
    statusBar()->showMessage(
        QStringLiteral("%1 × %2 × %3 · элементов: %4 · 0: %5 · 1: %6")
            .arg(length_)
            .arg(width_)
            .arg(height_)
            .arg(static_cast<qulonglong>(zeros_ + ones_))
            .arg(static_cast<qulonglong>(zeros_))
            .arg(static_cast<qulonglong>(ones_)));
}
