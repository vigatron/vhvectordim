# CubeView

A 3D scene widget built on `QOpenGLWidget`. It renders the cuboid as a set of
individual cubes in an isometric orthographic projection.

## API

```cpp
class CubeView : public QOpenGLWidget,
                 protected QOpenGLFunctions_3_3_Core {
public:
    explicit CubeView(QWidget *parent = nullptr);

    ~CubeView() override;

    void setData(std::vector<quint8> data,
                 int length, int width, int height);

    void setLayout(int length, int width, int height);

    void resetCamera();

    void setColors(const QColor &color0, const QColor &color1);
    void setTransparency0(int percent);
    void setTransparency1(int percent);
    void setBackgroundColor(const QColor &color);

    QColor color0() const;
    QColor color1() const;
    int transparency0() const;
    int transparency1() const;
    QColor backgroundColor() const;
};
```

`setData()` may be called before the OpenGL context exists: the data is stored
and the GPU buffers are filled during initialization. A repeated call refreshes
the scene. `setLayout()` changes the layout using the already loaded data; it is
invoked when a variant is selected in the right-hand panel. `setColors()` sets
the colors of elements `0` and `1`, while `setTransparency0()` and
`setTransparency1()` set the transparency of the corresponding cube type as a
percentage from `0` to `100`. `setBackgroundColor()` sets the scene background
color; it is white by default (`255/255/255`).

## Scene format

- Axes: `x` — length, `y` — width, `z` — height.
- Element index: `x + length * (y + width * z)`.
- The center of the cuboid coincides with the origin.
- Cube size is **80%** of the full cell (`0.80`), which keeps the seams
  visible.
- Default color `0`: `26/255` (10%), almost black.
- Default color `1`: `230/255` (90%), almost white.
- The transparency of `0` and `1` cubes is configured separately; both types
  default to 50% (alpha `0.5`).
- Scene background — `colorb`, white by default (`255/255/255`).

## Rendering

- OpenGL 3.3 Core.
- 36 vertices per cube: 6 faces of 2 triangles each.
- Position, color and cube type flag are supplied by the instance buffer:
  `x, y, z, r, g, b, isOne`.
- Vertex attributes:
  - 0 — position;
  - 1 — normal;
  - 2 — instance offset;
  - 3 — instance color;
  - 4 — the `isOne` flag used to select transparency.
- Depth test, back-face culling and MSAA are enabled.
- Lighting: a directional light with brightness `0.65 + 0.35 * diffuse`.
- The background color is held in `backgroundColor_` and applied through
  `glClearColor()` before each frame.

## Semi-transparency

- Blending `SRC_ALPHA / ONE_MINUS_SRC_ALPHA` is enabled.
- Depth writes for cubes are disabled so that interior layers remain visible.
- Transparency is set separately for `0` and `1` cubes as a percentage:
  `0` — opaque, `100` — fully invisible. Both types default to 50%
  (alpha `0.5`).
- Cubes are sorted from back to front along the view direction and uploaded to
  the instance buffer in that order.
- Sorting is repeated on camera rotation, data change or layout change.
- Zooming does not change the order and does not trigger sorting.

## Background

- The background color is `colorb`, white `255/255/255` by default.
- `setBackgroundColor()` changes the color and requests a repaint.
- The color is applied through `glClearColor()` before every rendered frame.

## Camera

- Orthographic projection.
- Initial direction: yaw `-45°`, pitch `35.264°` — classic isometric.
- The scene radius is derived from the cuboid diagonal.
- Zoom varies within `0.2 … 10`.

## Interaction

| Action | Result |
|---|---|
| Left button + drag | rotate the camera |
| Wheel | zoom |
| Double click | `resetCamera()` |

`resetCamera()` restores the initial angles and scale.

## Error handling

If the shaders fail to compile or link, a message is written to the log and the
scene stays empty. If the dimensions do not match the amount of data, no
instances are created.
