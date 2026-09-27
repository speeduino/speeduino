"""Compile the actual controller with fake MCU registers; no connected ECU required."""
from pathlib import Path
import os,re,subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[1]
compiler=shutil.which('g++')
if not compiler: raise SystemExit('g++ must be on PATH')
with tempfile.TemporaryDirectory(prefix='output-bench-tests-') as temp:
    d=Path(temp)
    # Only replace MCU/framework includes. Production function bodies stay intact.
    source=(root/'speeduino/injector_bench.cpp').read_text()
    source=re.sub(r'^#include .*\n','',source,flags=re.M)
    header='\n'.join('#include "'+str(root/f).replace('\\','/')+'"' for f in [
        'tools/tests/bench_host/stubs.h','speeduino/injector_bench.h','speeduino/injector_bench_logic.h','speeduino/idle_bench_logic.h','speeduino/bench_output_pin.h'])+'\n'
    macros=''
    (d/'test.cpp').write_text(header+source+macros+(root/'tools/tests/bench_host/cases.cpp').read_text())
    exe=d/'bench-tests.exe'
    subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-O2',str(d/'test.cpp'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)

    for inj,ign in [(4,5),(6,3),(8,1),(8,8)]:
        (d/'test.cpp').write_text(header+source+(root/'tools/tests/bench_host/portable_cases.cpp').read_text())
        subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-O2',
                        '-DINJ_CHANNELS='+str(inj),'-DIGN_CHANNELS='+str(ign),'-DMC33810_SUPPORT',
                        str(d/'test.cpp'),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
