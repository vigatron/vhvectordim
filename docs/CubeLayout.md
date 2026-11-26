# CubeLayout

The class determines the dimensions of a rectangular cuboid for a linear array.

## API

```cpp
class CubeLayout {
public:
    struct Dimensions {
        int length;
        int width;
        int height;
    };

    // All variants, the best one first.
    static std::vector<Dimensions> variants(std::size_t elementCount);

    // The best variant, or std::nullopt.
    static std::optional<Dimensions> compute(std::size_t elementCount);
};
```

`variants()` returns an empty list when the array cannot be arranged.
`compute()` returns `std::nullopt` in that case.

## Rules

- `length * width * height == elementCount`;
- `length > 1`, `width > 1`, `height > 1`;
- all valid variants are returned, sorted from the most compact one;
- among all valid variants the most compact one is considered the best.

## Algorithm

Triples `a <= b <= c` are searched for:

1. `a` is iterated from `2` up to the cube root of `N`.
2. If `a` divides `N`, the remainder is `rest = N / a`.
3. `b` is iterated from `a` up to the square root of `rest`.
4. If `b` divides `rest`, then `c = rest / b`.
5. For each variant the surface area `2 * (a*b + a*c + b*c)` is computed.
6. All variants are sorted by `(area, c, b, a)`: smallest area first, then the
   most compact largest side.

Dimensions are returned in ascending order: `length <= width <= height`.
In the right-hand panel they are displayed as strings such as `8x8x16`.

## Failure conditions

- `elementCount < 8` — the smallest cuboid is `2 × 2 × 2`;
- `elementCount > INT_MAX` — dimensions are stored in `int`;
- the number has no factorization into three factors each greater than 1
  (for example, a prime number).

## Examples

| N | Best variant | All variants |
|---|---|---|
| 8 | `2 × 2 × 2` | `2x2x2` |
| 12 | `2 × 2 × 3` | `2x2x3` |
| 64 | `4 × 4 × 4` | `4x4x4`, `2x4x8`, `2x2x16` |
| 1024 | `8 × 8 × 16` | `8x8x16`, `4x16x16`, `4x8x32`, `4x4x64`, `2x16x32`, `2x8x64`, `2x4x128`, `2x2x256` |
| 7 | error | — |
| 9 | error | — |
| 27 | `3 × 3 × 3` | `3x3x3` |

## Complexity

The search is bounded by the cube root and by the divisors of the remainder.
For the `N <= INT_MAX` limit it runs quickly; memory is allocated for the list
of variants.
