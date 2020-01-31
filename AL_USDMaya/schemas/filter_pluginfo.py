import json
import sys

def main():
    if len(sys.argv) < 1:
        return

    pluginfoPath = sys.argv[1]

    # Filter the comments line first.
    content = ''
    for line in open(pluginfoPath):
        if(not line.startswith('#')):
            content += line

    # Load and remove the SdfMetadata item
    loaded = json.loads(content)
    if 'SdfMetadata' in loaded['Plugins'][0]['Info'].keys():
        del loaded['Plugins'][0]['Info']['SdfMetadata']

    with open(pluginfoPath, 'w') as out:
        json.dump(loaded, out, indent=4)


if __name__ == '__main__':
    main()
