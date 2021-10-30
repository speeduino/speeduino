import more_itertools
from TsIniParser import TsIniParser, DataClassTransformer, Variable, Table
from typing import Union
from pathlib import Path

def _code_override(self:Union[Variable, Table]):
    """Extract the code_override directive if it exists"""
    def is_code_override(item):
        return isinstance(item, tuple) and item[0]=='code_override'

    if self.unknown_values:
        override = more_itertools.first_true(self.unknown_values, default=None, pred=is_code_override)
        if override:
            return override[1]
    return None

def _name_override(self:Union[Variable, Table]):
    """Extract the name override if it exists"""
    def is_name_override(override):
        return isinstance(override, tuple) and override[0]=='name_override'

    code_override = self.CodeOverride
    return code_override[1] if code_override and is_name_override(code_override) else None

def _var_unused(self:Variable):
    code_override = self.CodeOverride
    return code_override and isinstance(code_override, tuple) and "unused"==code_override[0]

def _var_code_name(self:Variable):
    """The name of the variable in the C source code. Accounts for over ride directives in the INI"""
    code_override = _name_override(self)
    return code_override if code_override else self.name

def _tab_code_name(self:Table):
    """The name of the table in the C source code. Accounts for over ride directives in the INI"""
    code_override = _name_override(self)
    return code_override if code_override else self.table_id

def ini_path():
    return Path(__file__).parent.parent / 'reference' / 'speeduino.ini'

def load_speeduino_ini():
    def add_speeduino_behaviors():
        # Add some Speeduino specific properties to the generic INI classes
        Variable.CodeOverride = property(_code_override)
        Table.CodeOverride = property(_code_override)
        Variable.Unused = property(_var_unused)
        Variable.CodeName = property(_var_code_name)
        Table.CodeName = property(_tab_code_name)

    # Lazy load, parse & transform the INI file *once* 
    if not hasattr(load_speeduino_ini, "speeduino_ini"):
        add_speeduino_behaviors()
        
        parser = TsIniParser()
        with open(ini_path(), 'r') as file:  
            parsetree = parser.parse(file)
            load_speeduino_ini.speeduino_ini = DataClassTransformer().transform(parsetree)

    return load_speeduino_ini.speeduino_ini