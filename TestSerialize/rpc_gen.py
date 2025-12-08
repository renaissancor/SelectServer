#!/usr/bin/env python3
"""Execute stub_gen and proxy_gen"""
import subprocess
import sys

print('=== RPC Code Generator ===\n')

result1 = subprocess.run([sys.executable, 'stub_gen.py'])
result2 = subprocess.run([sys.executable, 'proxy_gen.py'])

if result1.returncode == 0 and result2.returncode == 0:
    print('\nAll done!')
else:
    print('\nError occurred!')
    sys.exit(1)