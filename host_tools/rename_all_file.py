#! env python


import os
import json

def main() -> None:
    all_files:list[dict] = None
    with open('builddir/compile_commands.json') as f:
        all_files = json.load(f)

    for file in all_files:
        filepath:str = file['file']
        filepath = filepath.removeprefix('../')
        filepath = os.path.abspath(filepath)

        if os.path.exists(filepath):
            os.rename(filepath, filepath.removesuffix('.c')+'.cpp')
        else:
            print('{} doesn\'t exist, skip rename'.format(filepath))


if __name__ == '__main__':
    main()
