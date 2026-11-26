# CubeVisualizer

The main application window. It connects data loading, layout computation and
the 3D scene, and exposes the user commands.

## API

```cpp
class CubeVisualizer : public QMainWindow {
public:
    explicit CubeVisualizer(const QString &fileName,
                            std::vector<quint8> data,
                            std::vector<CubeLayout::Dimensions> variants,
                            QWidget *parent = nullptr);
};
```

## Window composition

- on the left, `CubeView` — 80% of the window width;
- on the right, a control panel 20% wide, containing from top to bottom:
  - `wdgVariants` — the list of layout variants;
  - `wdgColor` — three separate color selectors: for `color0`, `color1`
    and `colorb` (the 3D scene background color);
  - `wdgTrans` — two separate 0–100% transparency sliders for `color0` and
    `color1`, both at 50% by default;
  - `wdgMisc` — the `Save Screen` button;
- the File menu (`Файл`):
  - Open... (`Открыть...`, `Ctrl+O`);
  - Exit (`Выход`, `Ctrl+Q`);
- the View menu (`Вид`):
  - Reset Camera (`Сбросить камеру`, `R`);
- the status bar: dimensions, element count, counts of zeros and ones;
- a permanent interaction hint.

The minimum window size is 800 × 600, so the panel occupies at least 160 pixels
(20% of 800).

Runtime labels and messages are currently in Russian; the widget `objectName`s
(`wdgVariants`, `wdgColor`, `wdgTrans`, `wdgMisc`, `btnSaveScreen`) are
language-neutral and referenced by the documentation.

## Layout list

`populateLayoutList()` fills the `QListWidget` with rows such as `8x8x16`.
The first variant — the most compact one — is selected automatically. Selecting
another row triggers the `onLayoutSelected()` slot, which:

1. takes the dimensions from `variants_`;
2. calls `CubeView::setLayout()`;
3. updates the status bar.

The data is neither re-read nor copied at this point.

## Colors

`wdgColor` holds three buttons: for `color0`, `color1` and `colorb`. Pressing
one opens the standard `QColorDialog`. `color0`/`color1` are forwarded to
`CubeView::setColors()`, and `colorb` to `CubeView::setBackgroundColor()`.
The defaults are: `color0` almost black 10% (`26/255`), `color1` almost white
90% (`230/255`), `colorb` white 100% (`255/255/255`). Each button shows the
current color as a color swatch.

## Transparency

`wdgTrans` holds two horizontal sliders: one for `color0` cubes and one for
`color1` cubes. Each slider ranges from 0 to 100%. A value of 0 means opaque
cubes, 100 means fully invisible. Both sliders start at 50%. Changes are
forwarded to `CubeView::setTransparency0()` and
`CubeView::setTransparency1()`.

## Saving a screenshot

The `Save Screen` button in `wdgMisc` calls `CubeView::grabFramebuffer()`,
obtains an image of the 3D scene only and stores it as a JPEG file through
`QFileDialog`. The suggested name is
`<data-file-name>_YYYYMMDD_HHMMSS.jpg` in the directory of the loaded file.
If no extension is given, `.jpg` is appended.

## Opening a file

`openFile()`:

1. Shows a file selection dialog.
2. Loads and validates the file through `FileValidator`.
3. Computes all layouts through `CubeLayout::variants()`.
4. On failure shows a `QMessageBox` and keeps the current scene.
5. On success calls `applyData()`, fills the list and resets the camera.

## Applying data

`applyData()`:

- stores the file name and the list of layouts;
- selects the best layout;
- counts zeros and ones;
- passes the data to `CubeView::setData()`;
- fills the variant list;
- updates the window title and the status bar.

## Camera reset

`resetCamera()` delegates to `CubeView::resetCamera()`.

## Error behavior

- Invalid file: a dialog with the error text, the scene is left unchanged.
- Impossible layout: a dialog reporting the element count and the requirement
  that "all sides must be greater than 1"; the scene is left unchanged.
- Startup errors coming from the command line are handled in `main`: a message
  on stderr and return code `1`.
