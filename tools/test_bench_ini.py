"""Check generated bench fields belong to the actual Constants page."""
from pathlib import Path
import re
import subprocess
import sys
root = Path(__file__).resolve().parents[1]

s = (root/'reference/speeduino.ini').read_text()
section = None
page = None
found = {}
for line in s.splitlines():
    line = line.split(';',1)[0].strip()
    if re.fullmatch(r'\[\w+\]', line):
        section = line
    if line.startswith('page ='):
        page = int(line.split('=')[1])
    match = re.match(r'(benchChannel|benchOpen|benchPeriod|benchCount|coilChannel|coilDwell|coilPeriod|coilCount|idleBenchHome|idleBenchPosition|idleBenchDuty|auxFrequency)\s*=\s*(?:scalar|bits),\s*U16,\s*(\d+),', line)
    if match:
        assert section == '[Constants]', (match[1], section)
        assert page == 16, (match[1], page)
        found[match[1]] = int(match[2])
assert found == {'benchChannel':0, 'benchOpen':2, 'benchPeriod':4, 'benchCount':6, 'coilChannel':8, 'coilDwell':10, 'coilPeriod':12, 'coilCount':14, 'idleBenchHome':16, 'idleBenchPosition':18, 'idleBenchDuty':20, 'auxFrequency':22}, found
assert 'nPages              = 16' in s
for line in s.splitlines():
    key = line.strip().split('=')[0].strip()
    if key in ('pageIdentifier','pageReadCommand','pageValueWrite','pageChunkWrite','crc32CheckCommand','burnCommand'):
        values = re.findall(r'"([^"\n]*)"', line)
        assert len(values) == 16, (key, len(values))
        if key == 'burnCommand': assert values[-1] == ''
assert 'signature      = "speeduino 202504-outputtest1"' in s
print('PASS: twelve bench controls defined in Constants page 16; 16 command entries; no bench burn command')

assert "benchSettingsValid" in s
assert "(1 + (benchCount - 1) * benchPeriod + benchOpen) / 1000.0" in s

# TunerStudio resolves nested panels immediately, in definition order.
bench_dialogs = set()
for line in s.splitlines():
    dialog = re.match(r'\s*dialog\s*=\s*(\w+)', line)
    if dialog:
        bench_dialogs.add(dialog[1])
    panel = re.match(r'\s*panel\s*=\s*((?:bench|coil|aux|idleBench)\w+|injControls|injSettings|injPump|injActions|idleControls|idleSettings|idleActions|idleValuePanel|idleStatePanel|idleFeedback)', line)
    if panel:
        assert panel[1] in bench_dialogs, 'Forward panel reference: ' + panel[1]
print('PASS: every bench panel is defined before it is referenced')

assert 'subMenu = outputtest1' not in s
assert 'dialog = outputtest1' not in s
assert 'cmdEnableTestMode =' not in s
assert 'coilBench, "Coil Output Test"' in s
assert 'auxBench, "Auxiliary Output Test"' in s
for op in ('cmdCoilStart','cmdPumpOn','cmdPumpOff','cmdAux0Mode0','cmdAux7Mode1','cmdAux1Mode2'):
    assert re.search(r'^'+op+r'\s*=',s,re.M),op
page_sizes = re.search(r'^\s*pageSize\s*=([^\n]+)', s, re.M)[1]
assert page_sizes.strip().endswith(', 24')
print('PASS: new coil and auxiliary controls, 24-byte RAM page, old test menu removed')

for target in (0,6,7):
    assert f'cmdAux{target}Mode2' not in s
for target in (1,2,3,4):
    assert f'cmdAux{target}Mode2' in s
assert 'cmdAux0Mode1 = "N\\x06\\x00\\x01"' in s

assert 'idleBench, "Idle Valve Output Test"' in s
assert 'cmdIdleHome = "N\\x09\\x00"' in s
assert 'cmdIdleRun = "N\\x09\\x01"' in s
assert 'cmdIdleCycle = "N\\x09\\x02"' in s

for name in ('benchChannel','coilChannel'):
    line=re.search(r'^\s*'+name+r' = bits,.*$',s,re.M)[0]
    labels=re.findall(r'"([^"\n]*)"',line)
    assert labels == ['All available outputs']+list(map(str,range(1,9)))+['Configured outputs only']+['INVALID']*6
    assert 'noMsqSave = '+name in s
    assert 'controllerPriority = '+name in s
assert s.count('clickOnClose') >= 4
for kind in range(1,5):
    assert f'cmdEnable{kind} = "N\\x0A\\x{kind:02X}"' in s
assert 'dialog = injPump, "Fuel Pump", xAxis' in s
assert 'dialog = injActions, "Injector test", xAxis' in s
assert 'benchOwned && benchKind == 4 ? idleBenchRaw : 0' in s
print('PASS: All dropdowns, explicit Enable/Disable, grouped pump, gated idle telemetry')

for file,fn,pin in [('src/controllers/fan/fanController.cpp','fanInterrupt','pinFan'),('src/controllers/boost/boostController.cpp','boostInterrupt','pinBoost')]:
    body=(root/'speeduino'/file).read_text().split('void '+fn+'(void)')[1]
    assert body.lstrip().startswith('{\n  if(injectorBenchOwnsPin(pinNumbers.'+pin+')) return;')
assert 'PwmOutputChannel<BenchOutputPin<boardOutputPin_t>>' in (root/'speeduino/src/controllers/vvt/vvtController.cpp').read_text()
print('PASS: normal PWM interrupt interlocks')

for target,condition in {1:'fanEnable == 1 || fanEnable == 2',2:'boostEnabled',3:'vvtEnabled',4:'vvtEnabled && vvt2Enabled && !wmiEnabled',5:'wmiEnabled && !vvt2Enabled',6:'airConEnable',7:'airConEnable && airConFanEnabled'}.items():
    block=s.split(f'dialog = auxColumn{target},')[1].split('  dialog =')[0]
    assert f'&& ({condition})' in block
assert 'cmdAux5Mode2 = "N\\x06\\x05\\x02"' in s
assert 'cmdEnable4, {rpm == 0 && !benchOwned && idleBenchFeatureEnabled}' in s
print('PASS: WMI controls and per-feature menu interlocks')

assert 'BenchOutputPin<fastOutputPin_t> idle_pin, idle2_pin;' in (root/'speeduino/idle.cpp').read_text()
for target in (8,9):
    assert f'cmdAux{target}Mode2' in s
assert 'panel = auxIdleRow' in s

# Defaults must use displayed units, matching the controller reboot values.
for name,value in {'benchChannel':0,'benchOpen':1,'benchPeriod':100,'benchCount':100,
                   'coilChannel':0,'coilDwell':2.5,'coilPeriod':100,'coilCount':100}.items():
    actual=re.search(r'defaultValue = '+name+r', ([0-9.]+)',s)[1]
    assert float(actual)==value, (name,actual)
assert '"Position (steps) / duty (%)", ""' in s
assert '{idleBenchIsStepper ? "steps" : "%"}' not in s
