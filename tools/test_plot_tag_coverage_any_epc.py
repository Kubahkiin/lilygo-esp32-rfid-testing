"""Testy zliczania dowolnych EPC i interpretacji eksportu z firmware."""

import contextlib
import csv
import io
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import plot_tag_coverage_any_epc as coverage


class CoverageTests(unittest.TestCase):
    def test_sixty_arbitrary_epcs_include_tags_without_reliable_antenna(self):
        rows = ["epc,antenna_count,A2,A16"]
        for index in range(60):
            flag = int(index < 55)
            rows.append(f"E280ABCDEF{index:014X},{flag},{flag},0")
        antennas, tags = coverage.parse_csv_data("\n".join(rows))
        summary = coverage.build_summary(tags, 60)
        self.assertEqual(antennas, (2, 16))
        self.assertEqual(summary.observed_count, 60)
        self.assertEqual(summary.reliable_count, 55)
        self.assertIn("zgodna", summary.count_status)

    def test_deduplicates_case_but_preserves_full_epc_and_leading_zeros(self):
        _, tags = coverage.parse_csv_data(
            "epc,antenna_count,A5\n"
            " 00ab ,1,1\n00AB,1,1\nAB,1,1\n0001,0,0\n01,0,0"
        )
        self.assertEqual(list(tags), ["0001", "00AB", "01", "AB"])
        self.assertEqual(coverage.build_summary(tags, 60).observed_count, 4)

    def test_missing_excess_and_empty_table(self):
        for count, expected_status in [(58, "Brakuje 2"), (62, "Nadmiar: 2"), (0, "Brakuje 60")]:
            with self.subTest(count=count):
                rows = ["epc,antenna_count"]
                rows.extend(f"AB{index:04X},0" for index in range(count))
                _, tags = coverage.parse_csv_data("\n".join(rows))
                summary = coverage.build_summary(tags, 60)
                self.assertEqual(summary.observed_count, count)
                self.assertEqual(summary.reliable_count, 0)
                self.assertIn(expected_status, summary.count_status)

    def test_uses_only_last_csv_table_and_stops_at_end_marker(self):
        antennas, tags = coverage.parse_csv_data(
            "[RFID] TAG_CSV_BEGIN\nepc,antenna_count,A1\nAA,1,1\n"
            "[RFID] TAG_CSV_END\n[RFID] TAG_CSV_BEGIN\n"
            "epc,antenna_count,A3\nBB,0,0\n[RFID] TAG_CSV_END\nCC,1,1"
        )
        self.assertEqual(antennas, (3,))
        self.assertEqual(list(tags), ["BB"])

    def test_rejects_inconsistent_or_invalid_csv(self):
        samples = [
            "",
            "epc,antenna_count,A1,A01\nAA,2,1,1",
            "epc,antenna_count,A17\nAA,1,1",
            "epc,antenna_count,position\nAA,1,1",
            "epc,antenna_count,A1\nAA,0,1",
            "epc,antenna_count,A1\nAA,1,2",
            "epc,antenna_count,A1\nAA,1",
            "epc,antenna_count\nAA,-1",
            "epc,antenna_count\nAA,17",
            "epc,antenna_count\nAA,no",
            "epc,antenna_count\nXYZ,1",
            "epc,antenna_count,A1\nAA,1,1\naa,0,0",
        ]
        for sample in samples:
            with self.subTest(csv=sample), self.assertRaises(ValueError):
                coverage.parse_csv_data(sample)

    def test_expected_count_must_be_positive_integer(self):
        for value in [0, -1, 1.5, True, "60"]:
            with self.subTest(value=value), self.assertRaises(ValueError):
                coverage.build_summary({}, value)

    def test_main_generates_charts_and_mapping_for_sixty_empty_and_legacy(self):
        rows = ["epc,antenna_count,A1,A8"]
        rows.extend(
            f"E280{index:020X},{int(index < 55)},{int(index < 55)},0"
            for index in range(60)
        )
        samples = [
            ("\n".join(rows), 60, True),
            ("epc,antenna_count,A1,A8\n[RFID] TAG_CSV_END", 0, True),
            ("epc,antenna_count\nABCD,0", 1, False),
        ]
        for data, count, has_antennas in samples:
            with self.subTest(count=count), tempfile.TemporaryDirectory() as directory:
                output_dir = Path(directory)
                with (
                    patch.object(coverage, "CSV_DATA", data),
                    patch.object(coverage, "OUTPUT_DIR", output_dir),
                    patch.object(coverage, "SHOW_CHART", False),
                    contextlib.redirect_stdout(io.StringIO()) as stdout,
                    contextlib.redirect_stderr(io.StringIO()),
                ):
                    self.assertEqual(coverage.main(), 0)
                self.assertIn(f"{count}/60", stdout.getvalue())
                expected_files = {"tag_coverage.png", "tag_epc_mapping.csv"}
                if has_antennas:
                    expected_files |= {"antenna_tag_matrix.png", "antenna_tag_counts.png"}
                self.assertEqual({path.name for path in output_dir.iterdir()}, expected_files)
                for filename in expected_files - {"tag_epc_mapping.csv"}:
                    self.assertTrue((output_dir / filename).read_bytes().startswith(b"\x89PNG\r\n\x1a\n"))
                with (output_dir / "tag_epc_mapping.csv").open(newline="", encoding="utf-8") as file:
                    mapped_tags = list(csv.DictReader(file))
                self.assertEqual(len(mapped_tags), count)
                if count == 60:
                    self.assertEqual(mapped_tags[0]["epc"], "E28000000000000000000000")
                    self.assertEqual(mapped_tags[-1]["tag_index"], "60")


if __name__ == "__main__":
    unittest.main()
