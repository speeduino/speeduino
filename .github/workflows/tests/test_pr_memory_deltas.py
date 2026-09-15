import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "pr-memory-deltas.py"
spec = importlib.util.spec_from_file_location("memory_report", SCRIPT)
report = importlib.util.module_from_spec(spec)
spec.loader.exec_module(report)

TEENSY = """Processing teensy41 (platform: teensy)
teensy_size:   FLASH: code:20, data:3, headers:2   free for files:1000
teensy_size:    RAM1: variables:2, code:3, padding:4   free for local variables:2000
teensy_size:    RAM2: variables:5  free for malloc/new:3000
"""


class MemoryReportTests(unittest.TestCase):
    def parse(self, text):
        with tempfile.TemporaryDirectory() as folder:
            log = Path(folder) / "build.log"
            log.write_text(text, encoding="utf-8")
            return report.parse_multi_env_log(log)

    def test_teensy_uses_occupied_regions(self):
        self.assertEqual({'ram': 14, 'flash': 25}, self.parse(TEENSY)['teensy41'])

    def test_teensy_growth_has_positive_delta(self):
        base = self.parse(TEENSY)['teensy41']
        changed = TEENSY.replace('data:3', 'data:7').replace('files:1000', 'files:996')
        pr = self.parse(changed)['teensy41']
        self.assertIn('+4 B', report.format_delta_cols(base['flash'], pr['flash']))
        self.assertIn('-4 B', report.format_delta_cols(pr['flash'], base['flash']))

    def test_standard_teensy_format_and_real_zero(self):
        log = "Processing teensy35 (platform: teensy)\nRAM: [] 0.0% (used 0 bytes from 100 bytes)\nFlash: [=] 23.0% (used 23 bytes from 100 bytes)\n"
        self.assertEqual({'ram': 0, 'flash': 23}, self.parse(log)['teensy35'])

    def test_missing_metrics_are_unknown(self):
        self.assertEqual({'ram': None, 'flash': None}, self.parse('Processing board (platform: test)\n')['board'])
        self.assertIn('N/A', report.format_delta_cols(None, 10))
        self.assertIn('Unknown', report.format_delta_cols(10, None))
        self.assertNotIn('N/A', report.format_delta_cols(0, 0))

    def test_incomplete_teensy_ram_is_unknown(self):
        log = '\n'.join(line for line in TEENSY.splitlines() if 'RAM2:' not in line)
        self.assertIsNone(self.parse(log)['teensy41']['ram'])
        self.assertEqual(25, self.parse(log)['teensy41']['flash'])

    def test_ansi_color_codes(self):
        log = 'Processing board (platform: test)\n\x1b[32mRAM: [] 1.0% (used 1 bytes from 100 bytes)\x1b[0m\nFlash: [] 2.0% (used 2 bytes from 100 bytes)\n'
        self.assertEqual({'ram': 1, 'flash': 2}, self.parse(log)['board'])

    def test_missing_environment_does_not_get_a_zero_baseline(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder) / 'base.log'
            pr = Path(folder) / 'pr.log'
            base.write_text('', encoding='utf-8')
            pr.write_text(TEENSY, encoding='utf-8')
            env = dict(os.environ, PYTHONIOENCODING='utf-8')
            output = subprocess.check_output([sys.executable, str(SCRIPT), str(base), str(pr)], encoding='utf-8', env=env)
            self.assertIn('| N/A | 14 B | N/A | Unknown |', output)
            self.assertIn('| N/A | 25 B | N/A | Unknown |', output)


if __name__ == '__main__':
    unittest.main()
