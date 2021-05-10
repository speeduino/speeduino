import read_tsini
import gen_printpageascii
import os
import sys

Import("env")

def before_build():
    print("before_build: begin")

    iniFile = os.path.join(env['PROJECT_DIR'], 'reference', 'speeduino.ini')
    print(f'Using INI file {iniFile}')

    ts_ini_lines = read_tsini.read(iniFile)

    print_page_genfile = os.path.join(env['PROJECT_SRC_DIR'], 'pagePrintAscii.g.hpp')
    print(f'Generating {print_page_genfile}')
    with open(print_page_genfile, 'w') as f:
        gen_printpageascii.generate_printpageascii(ts_ini_lines, f)
    
    print("before_build: end")

before_build()