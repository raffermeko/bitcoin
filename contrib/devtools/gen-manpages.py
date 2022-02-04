#!/usr/bin/env python3
# Copyright (c) 2022 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
import os
import subprocess
import sys
import tempfile

BINARIES = [
'src/bitcoind',
'src/bitcoin-cli',
'src/bitcoin-tx',
'src/bitcoin-wallet',
'src/bitcoin-util',
'src/qt/bitcoin-qt',
]

# Paths to external utilities.
git = os.getenv('GIT', 'git')
help2man = os.getenv('HELP2MAN', 'help2man')

# If not otherwise specified, get top directory from git.
topdir = os.getenv('TOPDIR')
if not topdir:
    r = subprocess.run([git, 'rev-parse', '--show-toplevel'], stdout=subprocess.PIPE, check=True, universal_newlines=True)
    topdir = r.stdout.rstrip()

# Get input and output directories.
builddir = os.getenv('BUILDDIR', topdir)
mandir = os.getenv('MANDIR', os.path.join(topdir, 'doc/man'))

# Verify that all the required binaries are usable, and extract copyright
# message in a first pass.
copyright = None
versions = []
for relpath in BINARIES:
    abspath = os.path.join(builddir, relpath)
    try:
        r = subprocess.run([abspath, '--version'], stdout=subprocess.PIPE, universal_newlines=True)
    except IOError:
        print(f'{abspath} not found or not an executable', file=sys.stderr)
        sys.exit(1)
    # take first line (which must contain version)
    verstr = r.stdout.split('\n')[0]
    # last word of line is the actual version e.g. v22.99.0-5c6b3d5b3508
    verstr = verstr.split()[-1]
    assert verstr.startswith('v')

    # Only bitcoin-qt prints the copyright message on --version, so store it specifically.
    if relpath == 'src/qt/bitcoin-qt':
        copyright = r.stdout.split('\n')[1:]

    versions.append((abspath, verstr))

if any(verstr.endswith('-dirty') for (_, verstr) in versions):
    print("WARNING: Binaries were built from a dirty tree.")
    print('man pages generated from dirty binaries should NOT be committed.')
    print('To properly generate man pages, please commit your changes (or discard them), rebuild, then run this script again.')
    print()

with tempfile.NamedTemporaryFile('w', suffix='.h2m') as footer:
    # Create copyright footer, and write it to a temporary include file.
    assert copyright
    footer.write('[COPYRIGHT]\n')
    footer.write('\n'.join(copyright).strip())
    footer.flush()

    # Call the binaries through help2man to produce a manual page for each of them.
    for (abspath, verstr) in versions:
        outname = os.path.join(mandir, os.path.basename(abspath) + '.1')
        print(f'Generating {outname}…')
        subprocess.run([help2man, '-N', '--version-string=' + verstr, '--include=' + footer.name, '-o', outname, abspath], check=True)
