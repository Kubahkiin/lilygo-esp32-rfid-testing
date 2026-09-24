#!/usr/bin/env python3
"""Zlicza dowolne EPC i rysuje pokrycie bez przypisywania tagów do półek."""

from __future__ import annotations

import csv
import re
import sys
from dataclasses import dataclass
from pathlib import Path


# Wklej tabelę z Serial Monitora, opcjonalnie razem z TAG_CSV_BEGIN/END.
CSV_DATA = """

"""

# Liczba fizycznych tagów włożonych do testu, każdy z innym EPC.
EXPECTED_TAG_COUNT = 60
# Ustaw False, aby zapisać wykresy bez otwierania okien.
SHOW_CHART = True
# Podział wykresów dla czytelności; nie oznacza położenia tagów.
TAGS_PER_PANEL = 20
MAX_ANTENNAS = 16

OUTPUT_DIR = Path(__file__).resolve().parent / "output" / "any_epc"
EPC_PATTERN = re.compile(r"[0-9a-f]{2,}", re.IGNORECASE)
ANTENNA_COLUMN_PATTERN = re.compile(r"A(\d+)", re.IGNORECASE)


@dataclass(frozen=True)
class TagDetection:
    antenna_count: int
    antennas: frozenset[int] | None


@dataclass(frozen=True)
class CoverageSummary:
    expected_count: int
    observed_count: int
    reliable_count: int

    @property
    def count_status(self) -> str:
        difference = self.expected_count - self.observed_count
        if difference > 0:
            return f"Brakuje {difference} unikalnych EPC do oczekiwanej liczby."
        if difference < 0:
            return f"Nadmiar: {-difference} unikalnych EPC ponad oczekiwaną liczbę."
        return "Liczba unikalnych EPC zgodna z oczekiwaną."


def parse_csv_row(line: str) -> list[str]:
    return [cell.strip() for cell in next(csv.reader([line]))]


def parse_csv_data(
    csv_data: str,
) -> tuple[tuple[int, ...], dict[str, TagDetection]]:
    """Czyta ostatnią tabelę CSV; pełny EPC jest kluczem, bez konwersji na liczbę."""
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
            "Brak nagłówka epc,antenna_count. Wklej tabelę do CSV_DATA."
        )

    antenna_numbers: list[int] = []
    for column_name in header[2:]:
        match = ANTENNA_COLUMN_PATTERN.fullmatch(column_name)
        if not match:
            raise ValueError(f"Nieprawidłowa kolumna anteny: {column_name}.")
        antenna_number = int(match.group(1))
        if not 1 <= antenna_number <= MAX_ANTENNAS:
            raise ValueError(
                f"Numer anteny poza zakresem 1–{MAX_ANTENNAS}: {antenna_number}."
            )
        if antenna_number in antenna_numbers:
            raise ValueError(f"Powtórzona kolumna anteny A{antenna_number}.")
        antenna_numbers.append(antenna_number)

    detections_by_epc: dict[str, TagDetection] = {}
    for line in lines[header_index + 1 :]:
        if "TAG_CSV_END" in line:
            break
        cells = parse_csv_row(line)
        epc = cells[0].upper()
        if not EPC_PATTERN.fullmatch(epc):
            # Pozwala wkleić logi otaczające tabelę z Serial Monitora.
            if line.startswith("[") or len(cells) == 1:
                continue
            raise ValueError(f"Nieprawidłowy EPC w wierszu CSV: {cells[0]}.")
        if len(cells) != len(header):
            raise ValueError(
                f"Wiersz EPC {epc} ma {len(cells)} kolumn zamiast {len(header)}."
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
                raise ValueError(f"EPC {epc} ma flagę anteny inną niż 0 lub 1.")
            detected_antennas = frozenset(
                number
                for number, flag in zip(antenna_numbers, flags)
                if flag == "1"
            )
            if antenna_count != len(detected_antennas):
                raise ValueError(
                    f"EPC {epc}: antenna_count={antenna_count}, ale kolumny A* "
                    f"wskazują {len(detected_antennas)} anten."
                )

        detection = TagDetection(antenna_count, detected_antennas)
        previous = detections_by_epc.get(epc)
        if previous is not None and previous != detection:
            raise ValueError(
                f"EPC {epc} ma sprzeczne wiersze. Wklej jedną tabelę z testu."
            )
        # Identyczne wiersze oraz różnice wielkości liter nie zwiększają wyniku.
        detections_by_epc[epc] = detection

    # Pusta tabela z nagłówkiem jest prawidłowym wynikiem: 0 wykrytych EPC.
    return tuple(antenna_numbers), dict(sorted(detections_by_epc.items()))


def build_summary(
    detections_by_epc: dict[str, TagDetection], expected_count: int
) -> CoverageSummary:
    if type(expected_count) is not int or expected_count <= 0:
        raise ValueError("EXPECTED_TAG_COUNT musi być dodatnią liczbą całkowitą.")
    return CoverageSummary(
        expected_count=expected_count,
        # Firmware eksportuje także EPC odczytane choć raz z antenna_count=0.
        observed_count=len(detections_by_epc),
        reliable_count=sum(
            detection.antenna_count > 0 for detection in detections_by_epc.values()
        ),
    )


def save_epc_mapping(
    detections_by_epc: dict[str, TagDetection], antenna_numbers: tuple[int, ...]
) -> None:
    output_path = OUTPUT_DIR / "tag_epc_mapping.csv"
    with output_path.open("w", newline="", encoding="utf-8") as output_file:
        writer = csv.writer(output_file)
        writer.writerow(
            ["tag_index", "epc", "antenna_count"]
            + [f"A{number}" for number in antenna_numbers]
        )
        for index, (epc, detection) in enumerate(detections_by_epc.items(), start=1):
            writer.writerow(
                [index, epc, detection.antenna_count]
                + [
                    int(number in (detection.antennas or ()))
                    for number in antenna_numbers
                ]
            )
    print(f"Lista EPC i numerów porządkowych: {output_path.resolve()}")


def create_charts(
    detections_by_epc: dict[str, TagDetection],
    antenna_numbers: tuple[int, ...],
    summary: CoverageSummary,
) -> None:
    if type(TAGS_PER_PANEL) is not int or TAGS_PER_PANEL <= 0:
        raise ValueError("TAGS_PER_PANEL musi być dodatnią liczbą całkowitą.")
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

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    save_epc_mapping(detections_by_epc, antenna_numbers)
    indexed_tags = list(enumerate(detections_by_epc.values(), start=1))
    panels = [
        indexed_tags[start : start + TAGS_PER_PANEL]
        for start in range(0, len(indexed_tags), TAGS_PER_PANEL)
    ]
    figures = []

    def save_figure(figure, filename: str) -> None:
        output_path = OUTPUT_DIR / filename
        figure.savefig(output_path, dpi=180, bbox_inches="tight")
        print(f"Wykres zapisany: {output_path.resolve()}")
        figures.append(figure)

    coverage_figure, axes_grid = plt.subplots(
        nrows=1 + len(panels),
        figsize=(13, 3 + 2.7 * len(panels)),
        squeeze=False,
        constrained_layout=True,
    )
    coverage_figure.suptitle(
        f"Dowolne EPC: {summary.observed_count}/{summary.expected_count} "
        f"wykrytych przynajmniej raz\n{summary.count_status}",
        fontsize=15,
        fontweight="bold",
    )
    summary_axis = axes_grid[0, 0]
    counts = [summary.observed_count, summary.reliable_count]
    summary_axis.barh(
        [1, 0], counts, color=["#2563eb", "#f97316"], height=0.55
    )
    summary_axis.set_yticks(
        [1, 0], ["Odczytane choć raz", "Z anteną niezawodną"]
    )
    summary_axis.axvline(
        summary.expected_count, color="#475569", linestyle="--",
        label=f"Oczekiwane: {summary.expected_count}",
    )
    for position, count in zip([1, 0], counts):
        summary_axis.text(
            count + 0.3, position, str(count), va="center", fontweight="bold"
        )
    summary_axis.set_xlim(
        0, max(summary.expected_count, summary.observed_count) * 1.15 + 1
    )
    summary_axis.set_xlabel("Liczba unikalnych EPC")
    summary_axis.xaxis.set_major_locator(MaxNLocator(integer=True))
    summary_axis.legend(loc="lower right")
    axis_maximum = max(
        1, len(antenna_numbers),
        max((tag.antenna_count for tag in detections_by_epc.values()), default=0),
    ) + 1
    for axis, panel in zip(axes_grid[1:, 0], panels):
        indices = [index for index, _ in panel]
        values = [tag.antenna_count for _, tag in panel]
        bars = axis.bar(indices, values, color="#2563eb", width=0.72)
        for index, value in zip(indices, values):
            if value == 0:
                axis.axvspan(index - 0.45, index + 0.45, color="#fee2e2")
        axis.bar_label(bars, padding=3)
        axis.set_xticks(indices)
        axis.set_xlim(indices[0] - 0.6, indices[-1] + 0.6)
        axis.set_ylim(0, axis_maximum)
        axis.yaxis.set_major_locator(MaxNLocator(integer=True))
        axis.set_ylabel("Liczba anten\nniezawodnych")
        axis.grid(axis="y", linestyle="--", alpha=0.35)
        axis.set_axisbelow(True)
    coverage_figure.supxlabel(
        "Numery porządkowe EPC (sortowanie tekstowe, bez informacji o położeniu)\n"
        "0 = tag odczytany, ale żadna antena nie wykryła go w każdej próbie"
    )
    save_figure(coverage_figure, "tag_coverage.png")

    if antenna_numbers:
        color_map = ListedColormap(["#e5e7eb", "#f97316"])
        matrix_figure, matrix_axes = plt.subplots(
            nrows=max(1, len(panels)),
            figsize=(13, 1.5 + max(1, len(panels)) * (1.5 + len(antenna_numbers) * 0.3)),
            squeeze=False,
            constrained_layout=True,
        )
        matrix_figure.suptitle("Antena × EPC — wykrycia w każdej próbie", fontsize=15)
        if not panels:
            matrix_axes[0, 0].text(
                0.5, 0.5, "Brak odczytanych EPC", ha="center", va="center"
            )
            matrix_axes[0, 0].set_axis_off()
        for axis, panel in zip(matrix_axes[:, 0], panels):
            matrix = [
                [int(number in (tag.antennas or ())) for _, tag in panel]
                for number in antenna_numbers
            ]
            axis.imshow(
                matrix, aspect="auto", interpolation="nearest",
                cmap=color_map, vmin=0, vmax=1,
            )
            axis.set_xticks(range(len(panel)), [index for index, _ in panel])
            axis.set_yticks(
                range(len(antenna_numbers)), [f"A{number}" for number in antenna_numbers]
            )
            axis.set_xticks([i - 0.5 for i in range(1, len(panel))], minor=True)
            axis.set_yticks(
                [i - 0.5 for i in range(1, len(antenna_numbers))], minor=True
            )
            axis.grid(which="minor", color="white", linewidth=0.6)
            axis.tick_params(which="minor", bottom=False, left=False)
        matrix_axes[-1, 0].set_xlabel(
            "Numer porządkowy EPC (jak w tag_epc_mapping.csv)"
        )
        matrix_figure.legend(
            handles=[
                Patch(color="#f97316", label="Wykryty przez tę antenę w każdej próbie"),
                Patch(color="#e5e7eb", label="Brak odczytu w co najmniej jednej próbie"),
            ],
            loc="outside lower center", ncol=2,
        )
        save_figure(matrix_figure, "antenna_tag_matrix.png")

        antenna_figure, antenna_axis = plt.subplots(
            figsize=(10, max(4, 2 + len(antenna_numbers) * 0.4)),
            constrained_layout=True,
        )
        antenna_counts = [
            sum(number in (tag.antennas or ()) for tag in detections_by_epc.values())
            for number in antenna_numbers
        ]
        bars = antenna_axis.barh(
            [f"A{number}" for number in antenna_numbers], antenna_counts, color="#f97316"
        )
        antenna_axis.bar_label(bars, padding=3)
        antenna_axis.invert_yaxis()
        antenna_axis.axvline(
            summary.expected_count, color="#475569", linestyle="--",
            label=f"Oczekiwane: {summary.expected_count}",
        )
        antenna_axis.set_xlim(
            0, max(summary.expected_count, summary.observed_count) * 1.15 + 1
        )
        antenna_axis.xaxis.set_major_locator(MaxNLocator(integer=True))
        antenna_axis.set_xlabel("Liczba EPC wykrytych przez daną antenę w każdej próbie")
        antenna_axis.set_title("Niezawodny odczyt — porównanie anten")
        antenna_axis.legend(loc="lower right")
        save_figure(antenna_figure, "antenna_tag_counts.png")
    else:
        print(
            "CSV nie zawiera kolumn A1, A2… Pomijam wykresy konkretnych anten.",
            file=sys.stderr,
        )

    if SHOW_CHART:
        plt.show()
    for figure in figures:
        plt.close(figure)


def main() -> int:
    try:
        antenna_numbers, detections_by_epc = parse_csv_data(CSV_DATA)
        summary = build_summary(detections_by_epc, EXPECTED_TAG_COUNT)
        print(
            "Unikalne EPC wykryte przynajmniej raz: "
            f"{summary.observed_count}/{summary.expected_count} "
            f"({100 * summary.observed_count / summary.expected_count:.1f}%)."
        )
        print(summary.count_status)
        print(
            "EPC z co najmniej jedną anteną wykrywającą je w każdej próbie: "
            f"{summary.reliable_count}/{summary.expected_count}."
        )
        print(
            "EPC odczytane, ale bez anteny niezawodnej: "
            f"{summary.observed_count - summary.reliable_count}."
        )
        print(
            "Porównanie zakłada różne EPC wszystkich tagów i brak obcych tagów "
            "w zasięgu. EPC niewykrytych tagów oraz ich położenie są nieznane."
        )
        create_charts(detections_by_epc, antenna_numbers, summary)
    except (csv.Error, OSError, RuntimeError, ValueError) as exc:
        print(f"Błąd: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
