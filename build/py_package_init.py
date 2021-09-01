import subprocess
from pathlib import Path

Import("env")

python =env['PYTHONEXE']

subprocess.check_call([python, '--version'])
subprocess.check_call([python, '-m', 'pip', '--version'])

try:
    requirements = Path(__file__).parent / 'requirements.txt'
except NameError:  # We are the main py2exe script, not a module
    requirements = Path(env['PROJECT_DIR']) / 'build' / 'requirements.txt'

subprocess.check_call([python, '-m', 'pip', 'install', '-r', requirements])