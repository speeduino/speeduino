"""Compile the actual controller with fake MCU registers; no connected ECU required."""
from pathlib import Path
import os,re,subprocess,tempfile,shutil
root=Path(__file__).resolve().parents[1]
compiler=shutil.which('g++')
if not compiler: raise SystemExit('g++ must be on PATH')
def without_includes(source):
    return re.sub(r'^#(?:include|pragma once).*\n', '', source, flags=re.M)

def controller_function(source, name):
    # Use the real ISR/driver bodies, not a test implementation of the PWM logic.
    start = source.index('void '+name+'(void)')
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]+'\n'

def pwm_fixture():
    boost = (root/'speeduino/src/controllers/boost/boostController.cpp').read_text()
    fan = (root/'speeduino/src/controllers/fan/fanController.cpp').read_text()
    preamble = """
#define ATOMIC() if (true) // Single-threaded host test; no MCU interrupt masking.
#define PWM_FAN_AVAILABLE
using boardOutputPin_t = MockOutputPin;
using outputPin_t = MockOutputPin;
uint16_t pwmFreqToTicks(uint16_t hz) {return hz;}
uint16_t halfPercentage(uint8_t percent, uint16_t total) {return uint32_t(percent)*total/200;}
uint16_t boostCompare=0, boostCounter=0, fanCompare=0, fanCounter=0;
#define BOOST_TIMER_COMPARE boostCompare
#define BOOST_TIMER_COUNTER boostCounter
#define FAN_TIMER_COMPARE fanCompare
#define FAN_TIMER_COUNTER fanCounter
#undef SET_COMPARE
#define SET_COMPARE(compare, value) ((compare) = (value))
"""
    for path in ['src/pins/invertableOutputPin.h', 'src/pins/trackedOutputPin.h', 'src/pwm/PwmOutputChannel.h', 'src/pwm/interruptHandlers.h']:
        preamble += without_includes((root/'speeduino'/path).read_text())+'\n'
    # Keep production pin types: regressing to an unguarded pin must fail.
    preamble += re.search(r'^TESTABLE_STATIC (.+ boostOutput;)$', boost, re.M).group(1)+'\n'
    preamble += re.search(r'^using fanPwmChannel_t = .+;$', fan, re.M).group(0)+'\n'
    preamble += re.search(r'^TESTABLE_STATIC (.+ _fanPwm;)$', fan, re.M).group(1)+'\n'
    preamble += controller_function(fan, 'fanInterrupt')
    preamble += controller_function(boost, 'boostInterrupt')
    return preamble+(root/'tools/tests/bench_host/pwm_cases.cpp').read_text()

with tempfile.TemporaryDirectory(prefix='output-bench-tests-') as temp:
    d=Path(temp)
    # Only replace MCU/framework includes. Production function bodies stay intact.
    source=(root/'speeduino/injector_bench.cpp').read_text()
    source=re.sub(r'^#include .*\n','',source,flags=re.M)
    header='\n'.join('#include "'+str(root/f).replace('\\','/')+'"' for f in [
        'tools/tests/bench_host/stubs.h','speeduino/injector_bench.h','speeduino/injector_bench_logic.h','speeduino/idle_bench_logic.h','speeduino/bench_output_pin.h'])+'\n'
    macros=''
    cases=(root/'tools/tests/bench_host/cases.cpp').read_text()
    cases=cases.replace('int main() {', pwm_fixture()+'\nint main() {')
    (d/'test.cpp').write_text(header+source+macros+cases)
    exe=d/'bench-tests.exe'
    subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-O2',str(d/'test.cpp'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)

    for inj,ign in [(4,5),(6,3),(8,1),(8,8)]:
        (d/'test.cpp').write_text(header+source+(root/'tools/tests/bench_host/portable_cases.cpp').read_text())
        subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-O2',
                        '-DINJ_CHANNELS='+str(inj),'-DIGN_CHANNELS='+str(ign),'-DMC33810_SUPPORT',
                        str(d/'test.cpp'),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
