"""
Functions to parse a TunerStudio INI file.

The TunerStudio INI format doesn't follow the normal INI conventions.
E.g. sections can have duplicate keys, section contents are ordered.
"""

import re
import more_itertools
import itertools

# Common partial regex definitions
_regexComma = r'(?:\s*,\s*)' # Whitespaced comma
_keyRegEx = r'^\s*(?P<key>[^;].+)\s*=' # Ini file key. E.g.  '   foo  ='
_dataTypeRegex = fr'{_regexComma}(?P<type>[S|U]\d+)' # Comma prefixed data type. E.g. ' , S16 ' 
_fieldOffsetRegex = fr'{_regexComma}(?P<offset>\d+)' # Comma prefixed number
_otherRegEx = fr'({_regexComma}(?P<other>.+))*' # Comma prefixed anything

class Comment:
    """A comment line"""
    REGEX = re.compile(fr'^\s*(;.*)$')

    def __init__(self, match):
        self.Comment = match.group(1).strip()

    def __str__(self):
        return self.Comment

class Section:
    """A section line"""
    REGEX = re.compile(fr'^\s*\[(.+)\]\s*$')
        
    def __init__(self, match):
        self.Section = match.group(1).strip()

    def __str__(self):
        return self.Section

class FieldBase:
    """Base class for all field types"""
    __types = {
        'S08' : [ 'int8_t', 1 ],
        'S16' : [ 'int16_t', 2 ],
        'U08' : [ 'uint8_t', 1 ],
        'U16' : [ 'uint16_t', 2 ],
    }

    def __init__(self, match):
        self.Field = match.group('key').strip()
        self.DataType = FieldBase.__types[match.group('type').strip()]
        self.Offset = int(match.group('offset') or -1)
        self.Other = [x.strip() for x in (match.group('other') or "").split(',')]
        self.NameOverride = "use_name_parse" in self.Other

class ScalarField(FieldBase):
    """A scalar field"""
    REGEX = re.compile(fr'{_keyRegEx}(?:\s*scalar){_dataTypeRegex}(?:{_fieldOffsetRegex})?{_otherRegEx}')

    def __init__(self, match):
        super().__init__(match)
        self.OffsetEnd = self.Offset

    def __str__(self):
        return f'{self.Field}=scalar,{self.DataType[0]}'
        
class BitField(FieldBase):
    """A bit field"""
    REGEX = re.compile(fr'{_keyRegEx}(?:\s*bits){_dataTypeRegex}(?:{_fieldOffsetRegex})?{_regexComma}(?:\[(?P<bitStart>\d+):(?P<bitEnd>\d+)\]){_otherRegEx}')

    def __init__(self, match):
        super().__init__(match)
        self.BitStart = int(match.group('bitStart'))
        self.BitEnd = int(match.group('bitEnd'))
        self.OffsetEnd = self.Offset

    def __str__(self):
        return f'{self.Field}=bit,{self.DataType[0]},[{self.BitStart}:{self.BitEnd}]'

class TwoDimArrayField(FieldBase):
    """A 2-dimensional array field"""
    REGEX = re.compile(fr'{_keyRegEx}(?:\s*array){_dataTypeRegex}(?:{_fieldOffsetRegex})?{_regexComma}(?:\[\s*(?P<xDim>\d+)x(?P<yDim>\d+)\s*\]){_otherRegEx}')

    def __init__(self, match):
        super().__init__(match)
        self.xDim = int(match.group('xDim'))
        self.yDim = int(match.group('yDim'))
        self.OffsetEnd = self.Offset + (self.xDim * self.yDim * self.DataType[1])

    def __str__(self):
        return f'{self.Field}=2d-array,{self.DataType[0]},[{self.xDim}x{self.yDim}]'

class OneDimArrayField(FieldBase):
    """A 1-dimensional array field"""
    REGEX = re.compile(fr'{_keyRegEx}(?:\s*array){_dataTypeRegex}(?:{_fieldOffsetRegex})?{_regexComma}(?:\[\s*(?P<length>\d+)\s*\]){_otherRegEx}')

    def __init__(self, match):
        super().__init__(match)
        self.Length = int(match.group('length'))
        self.OffsetEnd = self.Offset + ((self.Length-1) * self.DataType[1])

    def __str__(self):
        return f'{self.Field}=array,{self.DataType[0]},[{self.Length}]'

class StringDef:
    """A string definition"""
    REGEX = re.compile(fr'{_keyRegEx}(?:\s*string){_regexComma}(?P<encoding>.+){_regexComma}(?:\s*(?P<length>\d+))')

    def __init__(self, match):
        self.Field = match.group('key').strip()
        self.Encoding = match.group('encoding').strip()
        self.Length = int(match.group('length'))

    def __str__(self):
        return f'{self.Field}=string,[{self.Length}]'

class KeyValue:
    """A generic key-value pair"""
    REGEX = re.compile(fr'{_keyRegEx}\s*(?P<value>.*)\s*$')

    def __init__(self, match):
        self.Key = match.group('key').strip()
        self.Values = [x.strip() for x in (match.group('value') or "").split(',')]

    def __str__(self):
        return f'{self.Key}={self.Values}'

class Define:
    """A #define"""
    REGEX = re.compile(fr'^\s*#define\s+(?P<condition>.+)\s*=\s*(?P<value>.+)\s*$')

    def __init__(self, match):
        self.Condition = match.group('condition').strip()
        self.Value = match.group('value').strip()

    def __str__(self):
        return f'{self.Condition}={self.Value}'

class BeginIfdef:
    """A #if"""
    REGEX = re.compile(fr'^\s*#if\s+(?P<condition>.*)\s*$')

    def __init__(self, match):
        self.Condition = match.group('condition').strip()

    def __str__(self):
        return f'#if {self.Condition}' 

class IfDefElse:
    """A #else"""
    REGEX = re.compile(fr'^\s*#else\s*$')

    def __init__(self, match):
        pass

    def __str__(self):
        return f'#else'         

class EndIfdef:
    """A #endif"""
    REGEX = re.compile(fr'^\s*#endif\s*$')

    def __init__(self, match):
        pass

    def __str__(self):
        return f'#endif' 

class UnknownLine:
    """A line with no parser"""
    def __init__(self, line):
        self.Line = line

    def __str__(self):
        return self.Line

class BlankLine:
    """A line with text"""
        
def parse_tsini(iniFile):
    """Parses a TS ini file into a collection of objects. One object per line"""

    def process_line(line):
        ts_ini_regex_handlers = [
            Comment,
            Section,
            ScalarField,
            BitField,
            TwoDimArrayField,
            OneDimArrayField,
            StringDef,
            Define,
            BeginIfdef,
            IfDefElse,
            EndIfdef,
            KeyValue
        ]
        if str.isspace(line):
            return BlankLine()

        for line_type in ts_ini_regex_handlers:
            match = line_type.REGEX.match(line)
            if match:
                return line_type(match)

        return UnknownLine(line)

    with open(iniFile, 'r') as f:
        return [process_line(x) for x in f]

class IfDef:
    """A complete #if section"""
    def __init__(self, condition, iflines, elselines):
        self.Condition = condition
        self.IfLines = iflines
        self.ElseLines = elselines

def coalesce_ifdefs(lines):
    """
    Find #if/#else/#endif groups and collapse each one into a single object  
    """

    def coalesce_group(group):
        def coalesce_ifdef(iterable):
            ifdef = more_itertools.first(group)
            iflines, *elselines = more_itertools.split_at(
                            more_itertools.islice_extended(group, 1, -1), 
                            lambda item: isinstance(item, IfDefElse))
            return [IfDef(ifdef.Condition, iflines, elselines[0] if elselines else [])]

        if isinstance(more_itertools.first(group), BeginIfdef):
            return coalesce_ifdef(group)
        return group

    return more_itertools.collapse(
                map(coalesce_group, 
                    more_itertools.split_when(lines, 
                        lambda itemA, itemB: isinstance(itemB, BeginIfdef) or isinstance(itemA, EndIfdef)
                )))

def read(iniFile):
    """
    Read a TunerStudio file into a dictionary.

    INI section names become the dictionary keys. 
    #if/#else/#endif line groups are collapsed
    """

    # Parse lines
    lines = parse_tsini(iniFile)
    
    # This is only here to make section grouping code simpler
    if not isinstance(lines[0], Section):
        lines.insert(0, Section(re.match(Section.REGEX, '[None]')))

    # Collapse #if groups into one item
    ifGroups = coalesce_ifdefs(lines)

    # Group into sections
    groups = more_itertools.split_before(ifGroups, lambda item: isinstance(item, Section))
    return { item[0].Section: item[1:] for item in groups}


def group_overlapping(fields):
    """ 
    Group fields together that have overlapping offsets (addresses)

    The INI file allows the addresses of fields to overlap - even for scalar and array fields
    (bit fields obviously overlap)
    """
    def is_overlapping(field1, field2):
        def overlap_helper(field1, field2):
            return (field1.Offset >= field2.Offset) and (field1.Offset <= field2.OffsetEnd)
        return overlap_helper(field1, field2) or overlap_helper(field2, field1)

    class group_overlap:       
        def __init__(self, item):
            self.group_start = item

        def __call__(self, item):
            # We order by widest item first at any overlapping address
            # So we must compare subsequent items to that first item in the group.
            if is_overlapping(self.group_start, item):
                return False
            # No overlaap - start of new group
            else:
                self.group_start = item
                return True

    # The sorting here is critical for the group_overlap algorithm
    # Sort by start address ascending, then end address descending. I.e. for
    # any overlapping fields, put the widest field first
    #
    # We assume overlaps are distinct and complete. I.e.
    #  0-7, 6-9 is not allowed in the INI file
    fields = sorted(fields, key=lambda item: (item.Offset, -item.OffsetEnd))
    if fields:
        return more_itertools.split_before(fields, group_overlap(fields[0]))
    return fields

_PARSE_SUBS= [
    # Array index flag 
    [ re.compile('_i(?P<index>\d+)_'), '[\g<index>].' ]
]

def get_code_fieldname(item:FieldBase):
    """Some field names require additional processing in order to get
    the equivalent source code variable name"""
    name = item.Field
    if item.NameOverride:
        for sub in _PARSE_SUBS:
            name = sub[0].sub(sub[1], name)
    return name