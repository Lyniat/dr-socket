import os
import sys
from pathlib import Path

dr_directory = os.path.dirname(os.path.realpath(__file__))

def exit_with_error(error):
    sys.stdout.write(error + "\n")
    sys.stdout.flush()
    sys.exit(1)

while True:
    dr_exe_1 = Path(os.path.join(dr_directory, "dragonruby.exe"))
    dr_exe_2 = Path(os.path.join(dr_directory, "dragonruby"))
    if dr_exe_1.is_file() or dr_exe_2.is_file():
        break

    if str(Path(dr_directory).anchor) is str(Path(dr_directory).absolute()):
        exit_with_error("Dragon Ruby directory could not be found!")

    dr_directory = os.path.abspath(os.path.join(dr_directory, '..'))

include_directory = Path(os.path.join(dr_directory, "include"))
include_file = Path(os.path.join(include_directory, "dragonruby.h"))

if not include_file.is_file():
    exit_with_error('"dragonruby.h" could not be found!')

sys.stdout.write(str(include_directory))
sys.stdout.flush()
sys.exit(0)


