import sys
import subprocess
import pkg_resources

python = sys.executable

installed = [ pkg.key for pkg in pkg_resources.working_set ]

subprocess.check_call([python, '--version'])
subprocess.check_call([python, '-m', 'pip', '--version'])

required_packages = ["more-itertools"]
for package in required_packages:
    if package in installed:
        print(f'Package {package} already installed.')
    else:
        print(f"Installing: {package}")
        subprocess.check_call([python, '-m', 'pip', 'install', package])