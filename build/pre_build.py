import gen_printpageascii
from pathlib import Path
from TsIni_Speeduino import load_speeduino_ini, ini_path

Import("env")

# SCons builder that generates pagePrintAscii.g.hpp
def genprintpageascii_build_action(target, source, env):
    speeduino_ini = load_speeduino_ini()
    print(f'Generating {str(target[0])}')
    with open(target[0].get_abspath(), 'w') as f:
        gen_printpageascii.generate_printpageascii(speeduino_ini, f)

print_page_genfile = File(Path(env['PROJECT_SRC_DIR']) / 'pagePrintAscii.g.hpp')
print_page_file = File(Path(env['PROJECT_SRC_DIR']) / 'pagePrintAscii.cpp')
ini_file = File(ini_path())

#Setup a SCons Builder that generates the print page ASCII code
env.Append(BUILDERS = {'print_ascii_builder': Builder(action=genprintpageascii_build_action)})
# Setup dependencies for the builder
env.print_ascii_builder(print_page_genfile, ini_file)

# Finally make the non-generated file dependent on the generated file
env.Depends(print_page_file, print_page_genfile)