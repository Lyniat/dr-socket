import sys
import yaml
import shutil


if len(sys.argv) > 3:
    config = sys.argv[1] #config
    src = sys.argv[2] #src
    dest = sys.argv[3] #dest

    if src[len(config) - 1] != "/":
        config = config + "/"

    if src[len(src) - 1] != "/":
        src = src + "/"

    if dest[len(dest) - 1] != "/":
        dest = dest + "/"

with open(config + 'install.yaml', 'r') as install_config:
    configuration = yaml.safe_load(install_config)
    print(configuration)
    if "file" in configuration:
        files = configuration["file"]
        for to_copy in files:
            if len(to_copy) != 2:
                continue
            if to_copy[0][len(to_copy[0]) - 1] == "/":
                continue
            if to_copy[1][len(to_copy[1]) - 1] == "/":
                continue
            shutil.copy2(src + to_copy[0], dest + to_copy[1])

    if "directory" in configuration:
        directories = configuration["directory"]
        for to_copy in directories:
            if len(to_copy) != 2:
                continue
            if to_copy[0][len(to_copy[0]) - 1] != "/":
                continue
            if to_copy[1][len(to_copy[1]) - 1] != "/":
                continue
            shutil.copytree(src + to_copy[0], dest + to_copy[1], dirs_exist_ok=True)

