#!/usr/bin/env python
import sys
import re

# This is a very dumb regex based method of translating from
# gn variable declarations and simple if statements to the
# equivalent cmake code. It only supports a few constructs and
# assumes the code is formatted a certain way. ANGLE gn files
# are auto-formatted so this should work most of the time.

# The output may need a bit of manual fixup, but it beats
# translating the whole thing by hand. If new constructs
# are added to the ANGLE gn files, hopefully we can just add
# a few extra regexes here.

if len(sys.argv) != 4:
    sys.stderr.write('Error: wrong number of arguments.\n\n')
    sys.stderr.write('Three arguments are required. The first argument is the path\n')
    sys.stderr.write('of the input .gni file. The second argument is the path of\n')
    sys.stderr.write('the output .cmake file. The third argument is a path to prepend\n')
    sys.stderr.write('to each file name (can be the empty string).\n')
    exit(1)

file = open(sys.argv[1], 'rb').read()

# Prepend path to every string.
file = re.sub(r'"((\\\"|[^"])+)"', '"' + sys.argv[3] + r'\1"', file)

# Remove import, assert, config, and angle_source_set statements.
file = re.sub(r'^\s*(import|assert|config|angle_source_set)\([^)]+\)( {.*})?$', r'', file, flags=re.MULTILINE|re.DOTALL)

# Translate gn single line list declaration:
file = re.sub(r'\[ ((?:"[^"]*",? )*)\]$', r' \1)', file, flags=re.MULTILINE)

# Translate gn single line list append:
file = re.sub(r'^(\s*)(\w+) \+= (\w+)$', r'\1 set(\2, "${\2};${\3}")', file, flags=re.MULTILINE)

# Translate gn list declaration:
# variable_name = [
#   "file/name/foo.cpp",
# ]
# to cmake list declaration:
# set(variable_name
#     "file/name/foo.cpp"
# )
file = re.sub(r'^(\s*)(\w+) = ?\[?', r'\1\1set(\2', file, flags=re.MULTILINE)
file = re.sub(r'^(\s*)("[^"]+"),$', r'\1\1\2', file, flags=re.MULTILINE)
file = re.sub(r'^(\s*)]$', r'\1\1)', file, flags=re.MULTILINE)

# Translate list append fom gn to cmake
file = re.sub(r'^(\s*)(\w+) \+= ?\[?', r'\1\1list(APPEND \2', file, flags=re.MULTILINE)

# Translate if statements from gn to cmake
file = re.sub(r'^(\s*)((?:} else )?)if \((.+)\) {$', r'\1\1\2if(\3)', file, flags=re.MULTILINE)
file = re.sub(r'^} else if$', r'elseif', file, flags=re.MULTILINE)
file = re.sub(r'^(\s*)} else {$', r'\1\1else()', file, flags=re.MULTILINE)
file = re.sub(r'^(\s*)}$', r'\1\1endif()', file, flags=re.MULTILINE)

# Translate logic ops from gn to cmake
file = re.sub(r' \|\| ', r' OR ', file, flags=re.MULTILINE)
file = re.sub(r' \&\& ', r' AND ', file, flags=re.MULTILINE)
file = re.sub(r' == ', r' STREQUAL ', file, flags=re.MULTILINE)
file = re.sub(r'!', r' NOT ', file, flags=re.MULTILINE)

out = open(sys.argv[2], 'wb')

out.write('# This file was generated with the command:\n')
out.write('# ' + ' '.join(['"' + arg.replace('"', '\\"') + '"' for arg in sys.argv]))
out.write('\n\n')
out.write(file)
