#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Windows 7 风格计算器 - 打包脚本
使用 PyInstaller 生成单文件 EXE
"""

import PyInstaller.__main__
import os
import sys


def build():
    """打包为单文件 EXE"""
    
    # 获取当前目录
    base_dir = os.path.dirname(os.path.abspath(__file__))
    script_path = os.path.join(base_dir, "calc_win7.py")
    
    # PyInstaller 参数
    args = [
        script_path,                          # 主脚本
        '--onefile',                          # 单文件模式
        '--windowed',                         # 无控制台窗口
        '--name=计算器_Win7风格',              # 输出文件名
        '--icon=NONE',                        # 无图标（可替换为 .ico 路径）
        '--clean',                            # 清理临时文件
        '--noconfirm',                        # 不确认覆盖
        # 隐藏导入
        '--hidden-import=PyQt5.sip',
        '--hidden-import=PyQt5.QtCore',
        '--hidden-import=PyQt5.QtGui',
        '--hidden-import=PyQt5.QtWidgets',
        '--hidden-import=dateutil',
        '--hidden-import=dateutil.relativedelta',
        '--hidden-import=calendar',
        # 优化
        '--strip',                            # 去除符号表
    ]
    
    print("=" * 60)
    print("开始打包 Windows 7 风格计算器...")
    print("=" * 60)
    print(f"源文件: {script_path}")
    print(f"输出目录: {os.path.join(base_dir, 'dist')}")
    print("-" * 60)
    
    try:
        PyInstaller.__main__.run(args)
        print("=" * 60)
        print("打包完成!")
        print(f"EXE 文件位置: {os.path.join(base_dir, 'dist', '计算器_Win7风格.exe')}")
        print("=" * 60)
    except Exception as e:
        print(f"打包失败: {e}")
        sys.exit(1)


if __name__ == "__main__":
    build()
