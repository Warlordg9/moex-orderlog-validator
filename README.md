# MOEX OrderLog Validator

[![CI](https://github.com/Warlordg9/moex-orderlog-validator/actions/workflows/ci.yml/badge.svg)](https://github.com/Warlordg9/moex-orderlog-validator/actions/workflows/ci.yml)
[![Docs](https://github.com/Warlordg9/moex-orderlog-validator/actions/workflows/docs.yml/badge.svg)](https://warlordg9.github.io/moex-orderlog-validator/)

**Документация (Doxygen):** https://warlordg9.github.io/moex-orderlog-validator/

Валидатор и нормализатор полного журнала заявок (OrderLog) Московской биржи. Программа
восстанавливает стакан по событиям отдельных заявок и ищет в данных аномалии: пересечения
стакана, переисполнения, ссылки на неизвестные заявки, дубликаты и некорректные цены.

Ядро детерминировано и работает только с файлами: скачивание данных из MOEX ISS вынесено
за пределы C++-ядра, поэтому сборка и тесты в CI не зависят от сети и биржи.

## Что делает программа

1. **Нормализация.** Цена переводится в целое число шагов цены (`int64`, без плавающей
   точки), время `HHMMSSmmm` / `HHMMSSuuuuuu` в наносекунды от полуночи.
2. **Движок стакана.** События `add / cancel / fill` превращаются в агрегированный стакан
   по уровням цен с лучшими bid/ask.
3. **Каталог правил с четырьмя исходами.** Каждое событие получает исход
   `pass / repair / quarantine / reject` и битовую маску флагов качества. Ничего не
   удаляется молча.

| Правило | Исход | Флаг |
|---|---|---|
| объём <= 0 | reject | `kInvalidVolume` |
| повторное добавление живой заявки | reject | `kDuplicateOrder` |
| отмена или сделка по неизвестной заявке | quarantine | `kUnknownOrder` |
| сделка больше остатка заявки | repair (обрезка до остатка) | `kOverfill` |
| отмена больше остатка заявки | repair (обрезка до остатка) | `kVolumeClamped` |
| после события bid >= ask | исход не меняется | `kCrossedBook` |

Строки CSV с неразбираемыми полями или ценой вне сетки шага не теряются: их номера
возвращаются в `ParseResult::rejected_lines`.

## Структура

```
include/olv/   публичные заголовки (types, quality, normalize, price_levels, order_book, orderlog_csv)
src/           реализация ядра (библиотека olv_core)
app/           CLI olv
tests/         unit-тесты GoogleTest + tests/data/sample_orderlog.csv (синтетический оракул)
bench/         бенчмарки Google Benchmark
docs/adr/      архитектурные решения (ADR)
.github/       CI (сборка, тесты, бенчмарк) и публикация документации на GitHub Pages
```

## Сборка и запуск

Нужны CMake >= 3.20 и компилятор с поддержкой C++20 (GCC 11+, Clang 14+).
GoogleTest и Google Benchmark скачиваются автоматически через `FetchContent`.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# тесты
ctest --test-dir build --output-on-failure

# бенчмарк
./build/bench/olv_bench

# CLI: файл OrderLog и шаг цены
./build/olv tests/data/sample_orderlog.csv 0.01
```

Формат входного CSV: `NO,SECCODE,BUYSELL,TIME,ORDERNO,ACTION,PRICE,VOLUME,TRADENO,TRADEPRICE`,
где `ACTION`: 0 означает отмену, 1 добавление, 2 сделку.

Пример вывода:

```
parsed events: 10, unparsable lines: 1

SECCODE       events    pass  repair  quarantine  reject  crossed  overfills
GAZP               1       1       0           0       0        0          0
SBER               9       6       1           1       1        2          1
```

## CI/CD

- `.github/workflows/ci.yml` запускается на каждый push и pull request: сборка через CMake,
  unit-тесты через `ctest`, бенчмарк (результат в журнале и в Job Summary), прогон CLI.
- `.github/workflows/docs.yml` запускается на каждый push в `main`: Doxygen собирает
  документацию, она публикуется на GitHub Pages.

## Roadmap

- Группировка событий в атомарные транзакции; пересечения стакана внутри одной транзакции
  перестанут считаться аномалией.
- Скрипт загрузки из MOEX ISS (сделки, свечи, справочник шага цены и лота) и сверка:
  доля сделок ISS, найденных по `TRADENO`, сверка объёма и OHLC по каждому тикеру.
- Время в наносекундах UTC с учётом торговой даты, тикер в числовой id.
- Выходы для агентов: чистые события с маской флагов, срезы стакана, бары по времени,
  объёму и обороту, `quality_report.json` и data contract.
- Эталонный тест на реальном дне: ровно 79 пересечений и 1 492 переисполнения.
- Хранение стакана массивом по шагам цены как третий вариант в ADR 0001.

Вне scope: real-time FAST/multicast и Arrow/Parquet. Для начала достаточно CSV и простого
бинарного формата; агенты прочитают его через DuckDB.
