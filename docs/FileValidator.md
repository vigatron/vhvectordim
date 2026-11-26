# FileValidator

The class is responsible for loading a binary file and validating its contents.

## API

```cpp
struct DataLoadResult {
    std::vector<quint8> bytes;
    QString error;
    bool ok() const;
};

class FileValidator {
public:
    static DataLoadResult load(const QString &fileName);
};
```

## Algorithm

1. The file is opened read-only.
2. The whole content is read into a `QByteArray`.
3. Open/read errors are detected.
4. An empty file is treated as an error.
5. Every byte is checked: only the values `0` and `1` are accepted.
6. On success, a `std::vector<quint8>` with the validated values is produced.

## Errors

On failure `error` holds a description and `bytes` is empty. The messages are
currently emitted in Russian:

| Message | Meaning |
|---|---|
| `Не удалось открыть файл ...` | failed to open the file |
| `Ошибка чтения файла ...` | file read error |
| `Файл пуст` | the file is empty |
| `Байт X в позиции Y не равен 0 или 1` | byte X at position Y is neither 0 nor 1 |

`result.ok()` is true only after a successful load.

## Result invariants

When `ok() == true`:

- `bytes` is non-empty;
- every element of `bytes` is either `0` or `1`;
- the data matches the file contents exactly.

## Notes

- The file is read once; the visualizer receives the ready-made array.
- The file size is not restricted at load time, but the subsequent layout step
  accepts sizes up to `INT_MAX` only.
