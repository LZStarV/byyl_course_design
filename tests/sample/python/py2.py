# Sample Python 2
import os
def read_and_write(path):
    if os.path.exists(path):
        return len(path)
    else:
        return 0

value = read_and_write('/tmp/file')

