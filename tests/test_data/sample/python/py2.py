# Sample Python 2
# Demo: import, filesystem check, function
import os
def read_and_write(path):
    # Return path length if it exists
    if os.path.exists(path):
        return len(path)
    else:
        # Otherwise return zero
        return 0

value = read_and_write('/tmp/file')
