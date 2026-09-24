#!/usr/bin/env python3
"""Tworzy wykres liczby anten, które wykryły poszczególne tagi RFID."""

from __future__ import annotations

import csv
import re
import sys
from dataclasses import dataclass
from pathlib import Path



CSV_DATA = """
epc,antenna_count,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16
0000,4,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0
0001,7,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,1
0002,6,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0
0003,6,1,1,0,1,1,0,0,0,0,0,0,0,1,1,0,0
0004,5,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0
0005,8,1,1,1,0,1,0,1,0,0,0,0,0,1,1,1,0
0006,11,1,1,1,1,1,1,0,1,0,0,0,0,1,1,1,1
0007,10,1,1,1,1,1,1,0,1,0,0,0,0,1,1,1,0
0008,11,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,0
0009,12,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0
0010,9,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0
0011,8,1,1,1,0,1,0,1,0,0,0,0,0,1,1,1,0
0012,6,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0
0013,9,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,0
0014,6,1,1,0,1,0,1,0,0,0,0,0,0,1,1,0,0
0015,9,1,1,1,1,1,0,1,0,0,0,0,0,1,1,1,0
0016,9,1,1,1,1,1,0,1,0,0,0,0,0,1,1,0,1
0017,10,1,1,1,0,1,1,1,1,0,0,0,0,1,1,1,0
0018,8,1,0,1,1,1,1,1,0,0,0,0,0,0,1,1,0
0019,7,1,1,0,1,0,1,1,0,0,0,0,0,1,1,0,0
0020,8,1,1,1,1,1,1,0,0,0,0,0,0,1,0,1,0
0021,7,0,1,1,0,1,1,1,0,0,0,0,0,0,1,1,0
0022,7,1,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0
0023,7,0,1,1,1,1,1,0,0,0,0,1,0,0,1,0,0
0024,8,1,1,1,1,1,0,0,0,0,0,1,0,1,1,0,0
0025,8,1,0,1,1,1,0,1,0,0,0,0,0,1,1,1,0
0026,6,1,1,1,1,0,0,0,0,0,0,0,0,1,0,1,0
0027,8,1,1,1,1,1,1,0,0,0,0,0,0,1,0,1,0
0028,8,0,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0
0029,10,1,1,1,1,1,1,0,0,1,0,1,0,1,1,0,0
0030,9,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1
0031,9,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0
0032,7,0,1,0,1,1,1,0,1,0,0,0,0,0,1,1,0
0033,10,1,1,1,1,1,1,0,1,0,1,0,0,0,1,0,1
0034,5,1,1,0,1,0,1,0,0,0,0,0,0,1,0,0,0
0035,7,1,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0
0036,7,1,1,1,1,0,0,1,0,0,0,0,0,1,0,1,0
0037,9,0,1,1,1,0,1,1,1,0,1,0,0,1,1,0,0
0038,9,0,1,1,1,1,1,0,1,0,0,0,0,1,1,1,0
0039,9,0,1,0,1,1,1,1,1,1,0,0,0,1,1,0,0
0040,9,1,0,1,0,1,1,1,0,1,1,0,0,0,0,1,1
0041,11,1,1,1,1,1,1,1,0,0,1,1,0,0,0,1,1
0042,9,0,1,1,0,1,1,1,1,0,0,0,0,1,0,1,1
0043,8,1,1,0,0,1,1,1,1,0,0,0,0,0,0,1,1
0044,9,1,1,1,1,1,1,0,0,1,0,0,0,1,0,0,1
0045,9,0,0,1,0,1,0,1,0,1,0,0,1,1,1,1,1
0046,13,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1
0047,13,0,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1
0048,10,1,1,1,1,1,1,0,1,0,1,1,0,0,0,0,1
0049,15,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1
0050,12,1,1,0,1,1,0,1,1,0,0,1,1,1,1,1,1
0051,7,0,0,1,0,1,1,1,1,0,0,0,0,0,0,1,1
0052,7,0,0,1,0,1,1,1,1,0,0,0,0,0,0,1,1
0053,7,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1
0054,7,0,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1
0055,5,0,0,1,0,0,0,0,0,1,0,0,0,1,0,1,1
0056,12,1,1,1,1,1,0,1,0,1,1,1,1,0,0,1,1
0057,11,0,1,1,0,1,1,1,1,1,1,1,0,0,0,1,1
0058,9,1,1,0,0,0,0,1,1,1,1,1,0,0,0,1,1
0059,10,0,0,1,1,0,1,0,1,1,1,0,1,0,1,1,1
0060,5,0,0,0,0,1,0,1,0,1,0,0,0,0,1,1,0
0061,9,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0
0062,10,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1
0063,6,0,0,0,0,0,1,1,0,0,1,0,1,0,0,1,1
0064,11,0,0,1,1,1,1,0,1,1,1,1,1,0,0,1,1
0065,5,0,0,0,0,1,1,1,0,1,0,1,0,0,0,0,0
0066,10,0,0,1,0,1,1,1,1,1,0,1,1,0,1,0,1
0067,12,0,1,1,1,1,1,1,1,1,1,1,0,0,1,0,1
0068,9,1,0,0,0,1,1,1,1,0,1,1,1,0,0,0,1
0069,9,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1
0070,5,0,0,0,0,0,0,1,0,1,1,1,0,0,0,0,1
0071,9,0,0,0,0,1,0,1,1,1,1,1,1,0,0,1,1
0072,6,0,0,0,0,1,0,1,1,1,1,1,0,0,0,0,0
0073,9,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,1
0074,7,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1
0075,7,0,0,0,0,0,1,0,1,1,0,1,1,0,0,1,1
0076,7,0,0,0,0,1,1,1,0,1,0,1,0,0,0,1,1
0077,7,0,0,0,0,1,1,1,1,1,0,1,0,0,0,0,1
0078,6,0,0,0,0,1,1,1,0,1,1,0,1,0,0,0,0
0079,8,0,0,0,0,1,1,0,1,1,1,1,1,0,0,0,1
0080,6,0,0,0,0,1,1,0,0,1,1,1,1,0,0,0,0
0081,9,0,0,0,1,1,1,0,1,1,1,1,1,0,0,0,1
0082,9,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1
0083,5,0,0,0,0,1,0,0,0,1,1,1,1,0,0,0,0
0084,8,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1
0085,6,0,0,0,0,1,1,1,0,1,0,1,1,0,0,0,0
0086,9,0,0,1,0,1,1,1,1,1,1,1,1,0,0,0,0
0087,7,0,0,0,0,0,1,1,0,1,1,1,1,0,0,0,1
0088,9,1,0,0,0,1,1,0,1,1,1,1,1,0,0,0,1
0089,8,0,0,0,0,1,1,0,1,1,1,1,1,0,0,0,1
0090,4,0,0,0,0,1,0,0,1,0,0,1,1,0,0,0,0
0091,5,0,0,0,0,0,0,0,1,1,1,0,1,0,0,0,1
0092,6,0,0,0,0,0,0,1,1,1,1,0,1,0,0,0,1
0093,7,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1
0094,4,0,0,0,0,1,0,0,0,1,1,0,1,0,0,0,0
0095,6,0,0,0,0,1,0,0,1,1,0,1,1,0,0,0,1
0096,6,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1
0097,10,0,0,1,0,1,1,1,1,1,1,1,1,0,0,0,1
0098,7,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1
0099,8,0,0,0,0,1,0,1,1,1,1,0,1,0,0,1,1
"""

# Ustaw False, jeżeli wykres ma zostać tylko zapisany, bez otwierania okna.
SHOW_CHART = True

MAX_ANTENNAS = 16
FIRST_TAG_NUMBER = 0
TAGS_PER_ROW = 10
ROWS_PER_SHELF = 2
TAGS_PER_SHELF = TAGS_PER_ROW * ROWS_PER_SHELF
SHELF_COUNT = 5
SHELF_COLORS = ("#2563eb", "#16a34a", "#fac44a", "#9333ea", "#aa6767")
SHELVES = tuple(
    (
        f"Półka {shelf_index + 1}",
        FIRST_TAG_NUMBER + shelf_index * TAGS_PER_SHELF,
        FIRST_TAG_NUMBER + (shelf_index + 1) * TAGS_PER_SHELF - 1,
        SHELF_COLORS[shelf_index % len(SHELF_COLORS)],
    )
    for shelf_index in range(SHELF_COUNT)
)
TOTAL_TAG_COUNT = SHELF_COUNT * TAGS_PER_SHELF
LAST_TAG_NUMBER = FIRST_TAG_NUMBER + TOTAL_TAG_COUNT - 1
# Kolejne numery od lewej do prawej: najpierw górny, potem dolny rząd.
SHELF_ROW_NAMES = ("Górny rząd", "Dolny rząd")
SHELF_ROWS = tuple(
    (
        shelf_name,
        row_name,
        first_tag + row_index * TAGS_PER_ROW,
        first_tag + (row_index + 1) * TAGS_PER_ROW - 1,
        shelf_color,
    )
    for shelf_name, first_tag, _, shelf_color in SHELVES
    for row_index, row_name in enumerate(SHELF_ROW_NAMES)
)

OUTPUT_PATH = (
    Path(__file__).resolve().parent / "output" / "tag_coverage.png"
)
ANTENNA_MATRIX_OUTPUT_PATH = (
    Path(__file__).resolve().parent
    / "output"
    / "antenna_tag_matrix.png"
)
ANTENNA_CHART_OUTPUT_DIR = (
    Path(__file__).resolve().parent / "output" / "antennas"
)
EPC_PATTERN = re.compile(r"[0-9a-f]{2,}", re.IGNORECASE)
ANTENNA_COLUMN_PATTERN = re.compile(r"A(\d+)", re.IGNORECASE)


@dataclass(frozen=True)
class TagDetection:
    antenna_count: int
    antennas: frozenset[int] | None


def parse_csv_row(line: str) -> list[str]:
    return [cell.strip() for cell in next(csv.reader([line]))]


def parse_csv_data(
    csv_data: str,
) -> tuple[tuple[int, ...], dict[str, TagDetection]]:
    """Parsuje dynamiczne kolumny A1, A2... wklejone do CSV_DATA."""
    lines = [line.strip() for line in csv_data.splitlines() if line.strip()]
    header_index: int | None = None
    header: list[str] = []

    for index, line in enumerate(lines):
        cells = parse_csv_row(line)
        if (
            len(cells) >= 2
            and cells[0].lower() == "epc"
            and cells[1].lower() == "antenna_count"
        ):
            header_index = index
            header = cells

    if header_index is None:
        raise ValueError(
            "Brak nagłówka CSV zaczynającego się od epc,antenna_count."
        )

    antenna_numbers: list[int] = []
    for column_name in header[2:]:
        match = ANTENNA_COLUMN_PATTERN.fullmatch(column_name)
        if not match:
            raise ValueError(
                f"Nieprawidłowa kolumna anteny w CSV: {column_name}."
            )
        antenna_number = int(match.group(1))
        if not 1 <= antenna_number <= MAX_ANTENNAS:
            raise ValueError(
                f"Numer anteny poza zakresem 1–{MAX_ANTENNAS}: "
                f"{antenna_number}."
            )
        if antenna_number in antenna_numbers:
            raise ValueError(
                f"Kolumna anteny A{antenna_number} występuje więcej niż raz."
            )
        antenna_numbers.append(antenna_number)

    detections_by_epc: dict[str, TagDetection] = {}
    for line in lines[header_index + 1 :]:
        if "TAG_CSV_END" in line:
            break

        cells = parse_csv_row(line)
        if not cells:
            continue
        epc = cells[0].upper()
        if not EPC_PATTERN.fullmatch(epc):
            continue
        if len(cells) != len(header):
            raise ValueError(
                f"Wiersz EPC {epc} ma {len(cells)} kolumn zamiast "
                f"{len(header)}."
            )

        try:
            antenna_count = int(cells[1])
        except ValueError as exc:
            raise ValueError(
                f"EPC {epc} ma nieprawidłową liczbę anten: {cells[1]}."
            ) from exc
        if not 0 <= antenna_count <= MAX_ANTENNAS:
            raise ValueError(
                f"EPC {epc} ma nieprawidłową liczbę anten: {antenna_count}."
            )

        detected_antennas: frozenset[int] | None = None
        if antenna_numbers:
            flags = cells[2:]
            if any(flag not in {"0", "1"} for flag in flags):
                raise ValueError(
                    f"EPC {epc} ma flagę anteny inną niż 0 lub 1."
                )
            detected_antennas = frozenset(
                antenna_number
                for antenna_number, flag in zip(antenna_numbers, flags)
                if flag == "1"
            )
            if antenna_count != len(detected_antennas):
                raise ValueError(
                    f"EPC {epc}: antenna_count={antenna_count}, ale "
                    f"kolumny A* wskazują {len(detected_antennas)} anten."
                )

        detections_by_epc[epc] = TagDetection(
            antenna_count=antenna_count,
            antennas=detected_antennas,
        )

    if not detections_by_epc:
        raise ValueError(
            "Brak danych. Wklej wiersze CSV do zmiennej CSV_DATA "
            "na początku skryptu."
        )
    return tuple(antenna_numbers), detections_by_epc


def epc_to_decimal_tag_number(epc: str) -> int | None:
    """Interpretuje np. 000...0050 jako numer taga 50, nie 0x50."""
    significant_digits = epc.lstrip("0") or "0"
    if not significant_digits.isdecimal():
        return None
    return int(significant_digits, 10)


def build_tag_detections(
    detections_by_epc: dict[str, TagDetection],
) -> tuple[dict[int, TagDetection], list[str]]:
    """Buduje skonfigurowany zakres tagów, uzupełniając braki zerami."""
    tag_detections = {
        tag_number: TagDetection(antenna_count=0, antennas=frozenset())
        for _, first_tag, last_tag, _ in SHELVES
        for tag_number in range(first_tag, last_tag + 1)
    }
    ignored_epcs: list[str] = []
    epc_by_tag_number: dict[int, str] = {}

    for epc, detection in detections_by_epc.items():
        tag_number = epc_to_decimal_tag_number(epc)
        if tag_number not in tag_detections:
            ignored_epcs.append(epc)
            continue

        previous_epc = epc_by_tag_number.get(tag_number)
        if previous_epc is not None and previous_epc != epc:
            raise ValueError(
                f"EPC {previous_epc} i {epc} wskazują ten sam tag "
                f"{tag_number}."
            )
        epc_by_tag_number[tag_number] = epc
        tag_detections[tag_number] = detection

    return tag_detections, ignored_epcs


def create_charts(
    tag_detections: dict[int, TagDetection],
    antenna_numbers: tuple[int, ...],
) -> None:
    try:
        import matplotlib
    except ImportError as exc:
        raise RuntimeError(
            "Brakuje pakietu matplotlib. Uruchom: "
            "python -m pip install -r tools/requirements.txt"
        ) from exc

    if not SHOW_CHART:
        matplotlib.use("Agg")

    import matplotlib.pyplot as plt
    from matplotlib.colors import ListedColormap
    from matplotlib.patches import Patch
    from matplotlib.ticker import MaxNLocator

    coverage_figure, axes_grid = plt.subplots(
        nrows=len(SHELF_ROWS),
        ncols=1,
        figsize=(12, 1.5 + SHELF_COUNT * 4.2),
        sharey=True,
        squeeze=False,
        constrained_layout=True,
    )
    axes = axes_grid[:, 0]
    coverage_figure.suptitle(
        "Niezawodne pokrycie tagów RFID przez anteny",
        fontsize=16,
        fontweight="bold",
    )
    coverage_figure.supxlabel("Numer taga")
    coverage_figure.supylabel(
        "Liczba anten, które wykryły tag w każdej próbie"
    )

    largest_value = max(
        (
            detection.antenna_count
            for detection in tag_detections.values()
        ),
        default=0,
    )
    axis_maximum = max(1, len(antenna_numbers), largest_value) + 1
    uncovered_tag_numbers = {
        tag_number
        for tag_number, detection in tag_detections.items()
        if detection.antenna_count == 0
    }

    for axis, (shelf_name, row_name, first_tag, last_tag, shelf_color) in zip(
        axes, SHELF_ROWS
    ):
        tag_numbers = list(range(first_tag, last_tag + 1))
        values = [
            tag_detections[tag_number].antenna_count
            for tag_number in tag_numbers
        ]
        colors = [
            shelf_color if value > 0 else "#d1d5db" for value in values
        ]
        for tag_number in tag_numbers:
            if tag_number in uncovered_tag_numbers:
                axis.axvspan(
                    tag_number - 0.45,
                    tag_number + 0.45,
                    facecolor="#fee2e2",
                    edgecolor="#b91c1c",
                    hatch="///",
                    linewidth=0.9,
                    alpha=0.55,
                    zorder=0,
                )
        bars = axis.bar(
            tag_numbers,
            values,
            width=0.72,
            color=colors,
            edgecolor="#334155",
            linewidth=0.7,
        )

        axis.set_title(
            f"{shelf_name} — {row_name.lower()}: tagi {first_tag}–{last_tag}"
        )
        axis.set_xticks(tag_numbers)
        axis.set_xlim(first_tag - 0.5, last_tag + 0.5)
        for tick_label, tag_number in zip(
            axis.get_xticklabels(),
            tag_numbers,
        ):
            if tag_number in uncovered_tag_numbers:
                tick_label.set_color("#b91c1c")
                tick_label.set_fontweight("bold")
        axis.set_ylim(0, axis_maximum)
        axis.yaxis.set_major_locator(MaxNLocator(integer=True))
        axis.grid(axis="y", linestyle="--", alpha=0.35)
        axis.set_axisbelow(True)

        for bar, value in zip(bars, values):
            label_y = value + 0.2 if value > 0 else 0.2
            axis.text(
                bar.get_x() + bar.get_width() / 2,
                label_y,
                str(value),
                ha="center",
                va="bottom",
                fontsize=9,
                fontweight="bold",
                color="#111827" if value > 0 else "#b91c1c",
            )

    if uncovered_tag_numbers:
        axes[0].legend(
            handles=[
                Patch(
                    facecolor="#fee2e2",
                    edgecolor="#b91c1c",
                    hatch="///",
                    label="Brak anteny niezawodnej",
                )
            ],
            loc="upper right",
        )

    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    coverage_figure.savefig(OUTPUT_PATH, dpi=180, bbox_inches="tight")
    print(f"Wykres zapisany: {OUTPUT_PATH.resolve()}")

    matrix_figure = None
    if antenna_numbers:
        detection_color_map = ListedColormap(["#e5e7eb", "#f97316"])

        matrix_height = max(6, 3 + len(antenna_numbers) * 0.8)
        matrix_width = max(12, SHELF_COUNT * 4.5)
        matrix_figure, matrix_axes = plt.subplots(
            nrows=ROWS_PER_SHELF,
            ncols=SHELF_COUNT,
            figsize=(matrix_width, matrix_height),
            squeeze=False,
            constrained_layout=True,
        )
        matrix_figure.suptitle(
            "Tagi wykryte przez anteny w każdej próbie",
            fontsize=15,
            fontweight="bold",
        )
        matrix_figure.supxlabel("Numer taga")
        matrix_figure.supylabel("Antena")
        for matrix_axis, (
            shelf_name,
            row_name,
            first_tag,
            last_tag,
            _,
        ) in zip(
            matrix_axes.T.flat,
            SHELF_ROWS,
        ):
            tag_numbers = list(range(first_tag, last_tag + 1))
            detection_matrix = [
                [
                    int(
                        tag_detections[tag_number].antennas is not None
                        and antenna_number
                        in tag_detections[tag_number].antennas
                    )
                    for tag_number in tag_numbers
                ]
                for antenna_number in antenna_numbers
            ]
            matrix_axis.imshow(
                detection_matrix,
                aspect="auto",
                interpolation="nearest",
                cmap=detection_color_map,
                vmin=0,
                vmax=1,
            )
            matrix_axis.set_title(
                f"{shelf_name} — {row_name.lower()}\n"
                f"tagi {first_tag}–{last_tag}",
                fontsize=11,
            )
            matrix_axis.set_xticks(range(TAGS_PER_ROW))
            matrix_axis.set_xticklabels(tag_numbers, fontsize=8)
            for tick_label, tag_number in zip(
                matrix_axis.get_xticklabels(),
                tag_numbers,
            ):
                if tag_number in uncovered_tag_numbers:
                    tick_label.set_color("#b91c1c")
                    tick_label.set_fontweight("bold")
            matrix_axis.set_yticks(range(len(antenna_numbers)))
            matrix_axis.set_yticklabels(
                [f"A{number}" for number in antenna_numbers]
            )
            for column_index, tag_number in enumerate(tag_numbers):
                if tag_number in uncovered_tag_numbers:
                    matrix_axis.axvspan(
                        column_index - 0.5,
                        column_index + 0.5,
                        facecolor="#fee2e2",
                        edgecolor="#b91c1c",
                        hatch="///",
                        linewidth=1,
                        alpha=0.4,
                    )
            matrix_axis.set_xticks(
                [index - 0.5 for index in range(1, TAGS_PER_ROW)],
                minor=True,
            )
            matrix_axis.set_yticks(
                [index - 0.5 for index in range(1, len(antenna_numbers))],
                minor=True,
            )
            matrix_axis.grid(
                which="minor",
                color="#ffffff",
                linewidth=0.6,
            )
            matrix_axis.tick_params(
                which="minor",
                bottom=False,
                left=False,
            )
        matrix_figure.legend(
            handles=[
                Patch(
                    color="#f97316",
                    label="Wykryty w każdej próbie",
                ),
                Patch(
                    color="#e5e7eb",
                    label="Pominięty w co najmniej jednej próbie",
                ),
                Patch(
                    facecolor="#fee2e2",
                    edgecolor="#b91c1c",
                    hatch="///",
                    label="Brak anteny niezawodnej",
                ),
            ],
            loc="outside lower center",
            ncol=3,
        )

        ANTENNA_MATRIX_OUTPUT_PATH.parent.mkdir(
            parents=True,
            exist_ok=True,
        )
        matrix_figure.savefig(
            ANTENNA_MATRIX_OUTPUT_PATH,
            dpi=180,
            bbox_inches="tight",
        )
        print(
            "Macierz anten zapisana: "
            f"{ANTENNA_MATRIX_OUTPUT_PATH.resolve()}"
        )

        ANTENNA_CHART_OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
        for old_chart in ANTENNA_CHART_OUTPUT_DIR.glob(
            "antenna_A*_tags.png"
        ):
            old_chart.unlink()

        for antenna_number in antenna_numbers:
            detected_tag_count = sum(
                detection.antennas is not None
                and antenna_number in detection.antennas
                for detection in tag_detections.values()
            )
            antenna_figure, antenna_axes_grid = plt.subplots(
                nrows=SHELF_COUNT,
                ncols=1,
                figsize=(12, 1.5 + SHELF_COUNT * 2.1),
                squeeze=False,
                constrained_layout=True,
            )
            antenna_axes = antenna_axes_grid[:, 0]
            antenna_figure.suptitle(
                f"Antena A{antenna_number}: niezawodnie wykryte tagi "
                f"({detected_tag_count}/{TOTAL_TAG_COUNT})",
                fontsize=15,
                fontweight="bold",
            )
            antenna_figure.supxlabel("W polu: numer taga oraz wynik wykrycia (1/0)")

            for axis, (
                shelf_name,
                first_tag,
                last_tag,
                _,
            ) in zip(antenna_axes, SHELVES):
                shelf_tag_numbers = [
                    list(range(row_start, row_start + TAGS_PER_ROW))
                    for row_start in range(
                        first_tag, last_tag + 1, TAGS_PER_ROW
                    )
                ]
                shelf_values = [
                    [
                        int(
                            tag_detections[tag_number].antennas is not None
                            and antenna_number
                            in tag_detections[tag_number].antennas
                        )
                        for tag_number in row_tag_numbers
                    ]
                    for row_tag_numbers in shelf_tag_numbers
                ]
                axis.imshow(
                    shelf_values,
                    origin="upper",
                    aspect="auto",
                    interpolation="nearest",
                    cmap=detection_color_map,
                    vmin=0,
                    vmax=1,
                )
                axis.set_title(
                    f"{shelf_name}: tagi {first_tag}–{last_tag}"
                )
                axis.set_xticks([])
                axis.set_yticks(range(ROWS_PER_SHELF))
                axis.set_yticklabels(SHELF_ROW_NAMES)
                axis.set_xticks(
                    [index - 0.5 for index in range(1, TAGS_PER_ROW)],
                    minor=True,
                )
                axis.set_yticks(
                    [index - 0.5 for index in range(1, ROWS_PER_SHELF)],
                    minor=True,
                )
                axis.grid(which="minor", color="#ffffff", linewidth=2)
                axis.tick_params(which="both", bottom=False, left=False)

                for row_index, (row_tag_numbers, row_values) in enumerate(zip(
                    shelf_tag_numbers,
                    shelf_values,
                )):
                    for column_index, (tag_number, detected) in enumerate(zip(
                        row_tag_numbers,
                        row_values,
                    )):
                        axis.text(
                            column_index,
                            row_index,
                            f"{tag_number}\n{detected}",
                            ha="center",
                            va="center",
                            fontsize=10,
                            fontweight="bold",
                            color="#ffffff" if detected else "#475569",
                        )

            antenna_figure.legend(
                handles=[
                    Patch(
                        color="#f97316",
                        label="Każda próba (1)",
                    ),
                    Patch(
                        color="#e5e7eb",
                        label="Co najmniej jeden brak (0)",
                    ),
                ],
                loc="outside lower center",
                ncol=2,
            )
            antenna_output_path = (
                ANTENNA_CHART_OUTPUT_DIR
                / f"antenna_A{antenna_number:02d}_tags.png"
            )
            antenna_figure.savefig(
                antenna_output_path,
                dpi=180,
                bbox_inches="tight",
            )
            plt.close(antenna_figure)

        print(
            f"Wykresy poszczególnych anten ({len(antenna_numbers)}) "
            f"zapisane w: {ANTENNA_CHART_OUTPUT_DIR.resolve()}"
        )
    else:
        print(
            "CSV ma stary format bez kolumn A1, A2... "
            "Pomijam wykres anten; wklej nowy wynik z ESP32.",
            file=sys.stderr,
        )

    if SHOW_CHART:
        plt.show()
    plt.close(coverage_figure)
    if matrix_figure is not None:
        plt.close(matrix_figure)


def main() -> int:
    try:
        antenna_numbers, detections_by_epc = parse_csv_data(CSV_DATA)
        tag_detections, ignored_epcs = build_tag_detections(
            detections_by_epc
        )

        detected_tag_count = sum(
            detection.antenna_count > 0
            for detection in tag_detections.values()
        )
        print(
            f"Wykryte tagi z zakresu "
            f"{FIRST_TAG_NUMBER}–{LAST_TAG_NUMBER}: "
            f"{detected_tag_count}/{TOTAL_TAG_COUNT}"
        )
        if ignored_epcs:
            print(
                f"Pominięte EPC spoza zakresu "
                f"{FIRST_TAG_NUMBER}–{LAST_TAG_NUMBER} "
                "lub zawierające A–F: "
                + ", ".join(ignored_epcs),
                file=sys.stderr,
            )

        create_charts(tag_detections, antenna_numbers)
    except (OSError, RuntimeError, ValueError) as exc:
        print(f"Błąd: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
