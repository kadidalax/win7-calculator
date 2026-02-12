#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Windows 7 Calculator - Build Script
Using PyInstaller to create single-file EXE
"""

import sys
import os

# Force UTF-8 encoding for Windows console
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

import PyInstaller.__main__


def build():
    """Build single-file EXE"""
    
    base_dir = os.path.dirname(os.path.abspath(__file__))
    script_path = os.path.join(base_dir, "calc_win7.py")
    
    args = [
        script_path,
        '--onefile',
        '--windowed',
        '--name=Calculator_Win7_Style',
        '--icon=NONE',
        '--clean',
        '--noconfirm',
        '--hidden-import=PyQt5.sip',
        '--hidden-import=PyQt5.QtCore',
        '--hidden-import=PyQt5.QtGui',
        '--hidden-import=PyQt5.QtWidgets',
        '--hidden-import=dateutil',
        '--hidden-import=dateutil.relativedelta',
        '--hidden-import=calendar',
        '--strip',
    ]
    
    print("=" * 60)
    print("Building Windows 7 Style Calculator...")
    print("=" * 60)
    print("Source: {}".format(script_path))
    print("Output: {}".format(os.path.join(base_dir, 'dist')))
    print("-" * 60)
    
    try:
        PyInstaller.__main__.run(args)
        print("=" * 60)
        print("Build completed successfully!")
        print("EXE location: {}".format(os.path.join(base_dir, 'dist', 'Calculator_Win7_Style.exe')))
        print("=" * 60)
    except Exception as e:
        print("Build failed: {}".format(str(e)))
        sys.exit(1)


if __name__ == "__main__":
    build()
