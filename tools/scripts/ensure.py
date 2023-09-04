import sys

package = sys.argv[1]

def test_import(package):
    try:
        __import__(package)
        sys.stdout.write("Found " + package + ".\n")
        sys.stdout.flush()
        sys.exit(0)
    except ImportError:
        sys.stdout.write("Could not find " + package + ".\n")
        sys.stdout.flush()
        sys.exit(1)

test_import(package)