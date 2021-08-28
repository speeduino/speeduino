import sys
import subprocess

python = sys.executable

subprocess.check_call([python, '--version'])
subprocess.check_call([python, '-m', 'pip', '--version'])

required_packages = [
   "https://github.com/adbancroft/TunerStudioIniParser/archive/refs/tags/v0.5.zip"
]
for install_path in required_packages:
    subprocess.check_call([python, '-m', 'pip', 'install', install_path])