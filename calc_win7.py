#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Windows 7 风格计算器
使用 PyQt5 实现，高度还原 Win7 玻璃质感界面
"""

import sys
import math
import calendar
from datetime import datetime, timedelta
from dateutil.relativedelta import relativedelta
from functools import partial
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLineEdit, QGridLayout,
                             QLabel, QFrame, QSizePolicy, QStackedWidget,
                             QCalendarWidget, QDateEdit, QSpinBox, QComboBox,
                             QTextEdit, QGroupBox, QTabWidget, QTableWidget,
                             QTableWidgetItem, QHeaderView)
from PyQt5.QtCore import Qt, QSize, pyqtSignal, QDate
from PyQt5.QtGui import QFont, QLinearGradient, QPalette, QColor, QIcon


class Win7Button(QPushButton):
    """Windows 7 风格按钮"""
    
    def __init__(self, text, parent=None, is_operator=False, is_memory=False, 
                 is_equal=False, is_special=False):
        super().__init__(text, parent)
        self.is_operator = is_operator
        self.is_memory = is_memory
        self.is_equal = is_equal
        self.is_special = is_special
        self.setMinimumSize(50, 40)
        self.setFont(QFont("Segoe UI", 11))
        self.update_style()
        
    def update_style(self):
        """应用 Win7 玻璃质感样式"""
        
        if self.is_equal:
            # 等号按钮 - 橙色渐变
            self.setStyleSheet("""
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FFCC99, stop:0.5 #FFB366, stop:1 #FF9933);
                    border: 1px solid #CC7A00;
                    border-radius: 3px;
                    color: #333;
                    padding: 5px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FFD9B3, stop:0.5 #FFC285, stop:1 #FFA64D);
                }
                QPushButton:pressed {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FF9933, stop:0.5 #FFB366, stop:1 #FFCC99);
                }
            """)
        elif self.is_memory:
            # 内存按钮 - 紫色系
            self.setStyleSheet("""
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #E6E6FA, stop:0.5 #D8D8F0, stop:1 #C8C8E8);
                    border: 1px solid #9999CC;
                    border-radius: 3px;
                    color: #333;
                    padding: 5px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #F0F0FF, stop:0.5 #E6E6FA, stop:1 #D8D8F0);
                }
                QPushButton:pressed {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #C8C8E8, stop:0.5 #D8D8F0, stop:1 #E6E6FA);
                }
            """)
        elif self.is_operator:
            # 运算符按钮 - 浅蓝渐变
            self.setStyleSheet("""
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #F0F8FF, stop:0.5 #E6F2FF, stop:1 #CCE5FF);
                    border: 1px solid #99CCFF;
                    border-radius: 3px;
                    color: #0066CC;
                    font-weight: bold;
                    padding: 5px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #F5FAFF, stop:0.5 #EBF5FF, stop:1 #D6EBFF);
                }
                QPushButton:pressed {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #CCE5FF, stop:0.5 #E6F2FF, stop:1 #F0F8FF);
                }
            """)
        elif self.is_special:
            # 特殊功能按钮 - 灰色系
            self.setStyleSheet("""
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #F5F5F5, stop:0.5 #EBEBEB, stop:1 #E0E0E0);
                    border: 1px solid #CCCCCC;
                    border-radius: 3px;
                    color: #333;
                    padding: 5px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FAFAFA, stop:0.5 #F5F5F5, stop:1 #EBEBEB);
                }
                QPushButton:pressed {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #E0E0E0, stop:0.5 #EBEBEB, stop:1 #F5F5F5);
                }
            """)
        else:
            # 数字按钮 - 银白渐变（Win7 玻璃感）
            self.setStyleSheet("""
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FFFFFF, stop:0.5 #F8F8F8, stop:1 #E8E8E8);
                    border: 1px solid #B8B8B8;
                    border-radius: 3px;
                    color: #333;
                    padding: 5px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FFFFFF, stop:0.5 #FAFAFA, stop:1 #F0F0F0);
                }
                QPushButton:pressed {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #E8E8E8, stop:0.5 #F0F0F0, stop:1 #F8F8F8);
                }
            """)


class CalendarWithWeekWidget(QWidget):
    """带周数的日历组件"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.init_ui()
        
    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(8)
        
        # 标题栏
        title = QLabel("📅 日历")
        title.setFont(QFont("Segoe UI", 12, QFont.Bold))
        title.setStyleSheet("color: #0066CC;")
        layout.addWidget(title)
        
        # 当前日期信息
        self.date_info = QLabel()
        self.date_info.setFont(QFont("Segoe UI", 10))
        self.date_info.setStyleSheet("""
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #E8F4FC, stop:1 #D6EBF5);
                border: 1px solid #B8D4E3;
                border-radius: 4px;
                padding: 8px;
                color: #333;
            }
        """)
        layout.addWidget(self.date_info)
        
        # 日历表格
        self.calendar_table = QTableWidget()
        self.calendar_table.setColumnCount(8)  # 周数 + 7天
        self.calendar_table.setHorizontalHeaderLabels(
            ["周数", "日", "一", "二", "三", "四", "五", "六"]
        )
        self.calendar_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.calendar_table.verticalHeader().setVisible(False)
        self.calendar_table.setSelectionMode(QTableWidget.SingleSelection)
        self.calendar_table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.calendar_table.setFixedHeight(220)
        self.calendar_table.setStyleSheet("""
            QTableWidget {
                background-color: #FAFAFA;
                border: 1px solid #CCCCCC;
                gridline-color: #E0E0E0;
            }
            QTableWidget::item {
                padding: 5px;
                text-align: center;
            }
            QTableWidget::item:selected {
                background-color: #3399FF;
                color: white;
            }
            QHeaderView::section {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #F0F8FF, stop:1 #CCE5FF);
                padding: 5px;
                border: 1px solid #99CCFF;
                font-weight: bold;
                color: #0066CC;
            }
        """)
        self.calendar_table.cellClicked.connect(self.on_date_selected)
        layout.addWidget(self.calendar_table)
        
        # 月份导航
        nav_layout = QHBoxLayout()
        
        self.prev_btn = Win7Button("◀ 上月", is_special=True)
        self.prev_btn.setFixedHeight(32)
        self.prev_btn.clicked.connect(self.prev_month)
        
        self.month_label = QLabel()
        self.month_label.setFont(QFont("Segoe UI", 11, QFont.Bold))
        self.month_label.setAlignment(Qt.AlignCenter)
        self.month_label.setStyleSheet("color: #333;")
        
        self.next_btn = Win7Button("下月 ▶", is_special=True)
        self.next_btn.setFixedHeight(32)
        self.next_btn.clicked.connect(self.next_month)
        
        self.today_btn = Win7Button("今天", is_operator=True)
        self.today_btn.setFixedHeight(32)
        self.today_btn.clicked.connect(self.go_today)
        
        nav_layout.addWidget(self.prev_btn)
        nav_layout.addWidget(self.month_label)
        nav_layout.addWidget(self.next_btn)
        nav_layout.addWidget(self.today_btn)
        
        layout.addLayout(nav_layout)
        
        # 初始化当前日期
        self.current_date = datetime.now()
        self.selected_date = None
        self.update_calendar()
        
    def update_calendar(self):
        """更新日历显示"""
        year = self.current_date.year
        month = self.current_date.month
        
        self.month_label.setText(f"{year}年 {month}月")
        
        # 更新日期信息
        week_num = self.current_date.isocalendar()[1]
        weekday = ["周一", "周二", "周三", "周四", "周五", "周六", "周日"][self.current_date.weekday()]
        self.date_info.setText(
            f"今天: {year}年{month}月{self.current_date.day}日 | "
            f"第{week_num}周 | {weekday} | "
            f"全年第{self.current_date.timetuple().tm_yday}天"
        )
        
        # 获取当月日历
        cal = calendar.Calendar(firstweekday=6)  # 周日为第一天
        month_days = cal.monthdayscalendar(year, month)
        
        self.calendar_table.setRowCount(len(month_days))
        
        for row, week in enumerate(month_days):
            # 周数（ISO标准）
            # 找到该周的第一个有效日期
            valid_days = [d for d in week if d != 0]
            if valid_days:
                # 使用周中的某一天计算周数
                sample_day = valid_days[0]
                sample_date = datetime(year, month, sample_day)
                iso_year, iso_week, _ = sample_date.isocalendar()
                week_num_item = QTableWidgetItem(f"{iso_week}")
                week_num_item.setTextAlignment(Qt.AlignCenter)
                week_num_item.setBackground(QColor("#E8F4FC"))
                week_num_item.setForeground(QColor("#0066CC"))
                week_num_item.setFont(QFont("Segoe UI", 9, QFont.Bold))
                self.calendar_table.setItem(row, 0, week_num_item)
            
            for col, day in enumerate(week):
                if day != 0:
                    item = QTableWidgetItem(str(day))
                    item.setTextAlignment(Qt.AlignCenter)
                    
                    # 标记今天
                    today = datetime.now()
                    if (year == today.year and month == today.month and day == today.day):
                        item.setBackground(QColor("#FF9933"))
                        item.setForeground(QColor("white"))
                        item.setFont(QFont("Segoe UI", 10, QFont.Bold))
                    # 标记周末
                    elif col in [0, 6]:  # 周日或周六
                        item.setForeground(QColor("#CC0000"))
                    
                    self.calendar_table.setItem(row, col + 1, item)
        
        # 调整行高
        for row in range(self.calendar_table.rowCount()):
            self.calendar_table.setRowHeight(row, 28)
            
    def prev_month(self):
        """上一个月"""
        self.current_date = self.current_date - relativedelta(months=1)
        self.update_calendar()
        
    def next_month(self):
        """下一个月"""
        self.current_date = self.current_date + relativedelta(months=1)
        self.update_calendar()
        
    def go_today(self):
        """回到今天"""
        self.current_date = datetime.now()
        self.update_calendar()
        
    def on_date_selected(self, row, col):
        """日期被选中"""
        if col == 0:  # 点击了周数列
            return
        item = self.calendar_table.item(row, col)
        if item:
            day = int(item.text())
            self.selected_date = datetime(
                self.current_date.year, 
                self.current_date.month, 
                day
            )
            week_num = self.selected_date.isocalendar()[1]
            weekday = ["周一", "周二", "周三", "周四", "周五", "周六", "周日"][self.selected_date.weekday()]
            self.date_info.setText(
                f"选中: {self.selected_date.year}年{self.selected_date.month}月{day}日 | "
                f"第{week_num}周 | {weekday} | "
                f"全年第{self.selected_date.timetuple().tm_yday}天"
            )


class DateCalculatorWidget(QWidget):
    """日期计算器组件"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.init_ui()
        
    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(12)
        
        # 标题
        title = QLabel("📊 日期计算")
        title.setFont(QFont("Segoe UI", 12, QFont.Bold))
        title.setStyleSheet("color: #0066CC;")
        layout.addWidget(title)
        
        # ===== 功能1: 日期间隔计算 =====
        diff_group = QGroupBox("计算两个日期之间的间隔")
        diff_group.setFont(QFont("Segoe UI", 10))
        diff_group.setStyleSheet("""
            QGroupBox {
                border: 2px solid #B8D4E3;
                border-radius: 6px;
                margin-top: 10px;
                padding-top: 10px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
                color: #0066CC;
            }
        """)
        
        diff_layout = QVBoxLayout(diff_group)
        
        # 开始日期
        start_layout = QHBoxLayout()
        start_layout.addWidget(QLabel("开始日期:"))
        self.start_date = QDateEdit()
        self.start_date.setCalendarPopup(True)
        self.start_date.setDate(QDate.currentDate())
        self.start_date.setStyleSheet("""
            QDateEdit {
                padding: 5px;
                border: 1px solid #CCCCCC;
                border-radius: 3px;
                background: white;
            }
        """)
        start_layout.addWidget(self.start_date)
        diff_layout.addLayout(start_layout)
        
        # 结束日期
        end_layout = QHBoxLayout()
        end_layout.addWidget(QLabel("结束日期:"))
        self.end_date = QDateEdit()
        self.end_date.setCalendarPopup(True)
        self.end_date.setDate(QDate.currentDate().addDays(7))
        self.end_date.setStyleSheet("""
            QDateEdit {
                padding: 5px;
                border: 1px solid #CCCCCC;
                border-radius: 3px;
                background: white;
            }
        """)
        end_layout.addWidget(self.end_date)
        diff_layout.addLayout(end_layout)
        
        # 计算按钮
        self.calc_diff_btn = Win7Button("计算间隔", is_operator=True)
        self.calc_diff_btn.setFixedHeight(36)
        self.calc_diff_btn.clicked.connect(self.calculate_difference)
        diff_layout.addWidget(self.calc_diff_btn)
        
        # 结果显示
        self.diff_result = QLabel("点击按钮计算日期间隔")
        self.diff_result.setFont(QFont("Segoe UI", 10))
        self.diff_result.setStyleSheet("""
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #F0F8FF, stop:1 #E6F2FF);
                border: 1px solid #99CCFF;
                border-radius: 4px;
                padding: 10px;
                color: #333;
                min-height: 60px;
            }
        """)
        self.diff_result.setWordWrap(True)
        diff_layout.addWidget(self.diff_result)
        
        layout.addWidget(diff_group)
        
        # ===== 功能2: 日期加减计算 =====
        add_group = QGroupBox("给定日期加减天数/周数")
        add_group.setFont(QFont("Segoe UI", 10))
        add_group.setStyleSheet("""
            QGroupBox {
                border: 2px solid #B8D4E3;
                border-radius: 6px;
                margin-top: 10px;
                padding-top: 10px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
                color: #0066CC;
            }
        """)
        
        add_layout = QVBoxLayout(add_group)
        
        # 基准日期
        base_layout = QHBoxLayout()
        base_layout.addWidget(QLabel("基准日期:"))
        self.base_date = QDateEdit()
        self.base_date.setCalendarPopup(True)
        self.base_date.setDate(QDate.currentDate())
        self.base_date.setStyleSheet("""
            QDateEdit {
                padding: 5px;
                border: 1px solid #CCCCCC;
                border-radius: 3px;
                background: white;
            }
        """)
        base_layout.addWidget(self.base_date)
        add_layout.addLayout(base_layout)
        
        # 数值和单位
        value_layout = QHBoxLayout()
        value_layout.addWidget(QLabel("数值:"))
        self.value_spin = QSpinBox()
        self.value_spin.setRange(-9999, 9999)
        self.value_spin.setValue(7)
        self.value_spin.setStyleSheet("""
            QSpinBox {
                padding: 5px;
                border: 1px solid #CCCCCC;
                border-radius: 3px;
                background: white;
            }
        """)
        value_layout.addWidget(self.value_spin)
        
        value_layout.addWidget(QLabel("单位:"))
        self.unit_combo = QComboBox()
        self.unit_combo.addItems(["天", "周", "月", "年"])
        self.unit_combo.setStyleSheet("""
            QComboBox {
                padding: 5px;
                border: 1px solid #CCCCCC;
                border-radius: 3px;
                background: white;
            }
        """)
        value_layout.addWidget(self.unit_combo)
        add_layout.addLayout(value_layout)
        
        # 计算按钮
        self.calc_add_btn = Win7Button("计算结果日期", is_operator=True)
        self.calc_add_btn.setFixedHeight(36)
        self.calc_add_btn.clicked.connect(self.calculate_addition)
        add_layout.addWidget(self.calc_add_btn)
        
        # 结果显示
        self.add_result = QLabel("点击按钮计算结果日期")
        self.add_result.setFont(QFont("Segoe UI", 10))
        self.add_result.setStyleSheet("""
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #F0F8FF, stop:1 #E6F2FF);
                border: 1px solid #99CCFF;
                border-radius: 4px;
                padding: 10px;
                color: #333;
                min-height: 50px;
            }
        """)
        self.add_result.setWordWrap(True)
        add_layout.addWidget(self.add_result)
        
        layout.addWidget(add_group)
        layout.addStretch()
        
    def calculate_difference(self):
        """计算日期间隔"""
        start = self.start_date.date().toPyDate()
        end = self.end_date.date().toPyDate()
        
        # 确保 start <= end
        if start > end:
            start, end = end, start
            swapped = True
        else:
            swapped = False
            
        delta = end - start
        days = delta.days
        
        # 计算周数
        weeks = days // 7
        remaining_days = days % 7
        
        # 计算工作日（简化版：只排除周末）
        workdays = 0
        current = start
        while current <= end:
            if current.weekday() < 5:  # 周一到周五
                workdays += 1
            current += timedelta(days=1)
        
        # 计算月数和年数（近似）
        start_dt = datetime.combine(start, datetime.min.time())
        end_dt = datetime.combine(end, datetime.min.time())
        rd = relativedelta(end_dt, start_dt)
        
        result_text = f"📊 日期间隔统计:\n\n"
        if swapped:
            result_text += f"⚠️ 已自动调换日期顺序\n\n"
        result_text += f"📅 总天数: {days} 天\n"
        result_text += f"📆 周数: {weeks} 周"
        if remaining_days > 0:
            result_text += f" + {remaining_days} 天"
        result_text += f"\n"
        result_text += f"📈 月数: {rd.years * 12 + rd.months} 个月"
        if rd.days > 0:
            result_text += f" + {rd.days} 天"
        result_text += f"\n"
        result_text += f"🗓️ 年数: {rd.years} 年"
        if rd.months > 0:
            result_text += f" {rd.months} 个月"
        if rd.days > 0:
            result_text += f" {rd.days} 天"
        result_text += f"\n\n"
        result_text += f"💼 工作日: 约 {workdays} 天\n"
        result_text += f"🎯 第{start.isocalendar()[1]}周 → 第{end.isocalendar()[1]}周"
        
        self.diff_result.setText(result_text)
        
    def calculate_addition(self):
        """计算日期加减"""
        base = self.base_date.date().toPyDate()
        value = self.value_spin.value()
        unit = self.unit_combo.currentText()
        
        base_dt = datetime.combine(base, datetime.min.time())
        
        if unit == "天":
            result = base_dt + timedelta(days=value)
        elif unit == "周":
            result = base_dt + timedelta(weeks=value)
        elif unit == "月":
            result = base_dt + relativedelta(months=value)
        elif unit == "年":
            result = base_dt + relativedelta(years=value)
            
        result_date = result.date()
        
        # 计算周数信息
        base_week = base.isocalendar()[1]
        result_week = result_date.isocalendar()[1]
        
        delta = result_date - base
        days_diff = delta.days
        
        result_text = f"📅 计算结果:\n\n"
        result_text += f"基准日期: {base.year}年{base.month}月{base.day}日 (第{base_week}周)\n"
        result_text += f"运算: {'+' if value >= 0 else ''}{value} {unit}\n\n"
        result_text += f"✅ 结果日期: {result_date.year}年{result_date.month}月{result_date.day}日\n"
        result_text += f"📆 第{result_week}周 | "
        weekday = ["周一", "周二", "周三", "周四", "周五", "周六", "周日"][result_date.weekday()]
        result_text += f"{weekday}\n"
        result_text += f"📊 间隔: {abs(days_diff)} 天"
        if abs(days_diff) >= 7:
            weeks = abs(days_diff) // 7
            rem = abs(days_diff) % 7
            result_text += f" (约 {weeks} 周"
            if rem > 0:
                result_text += f" {rem} 天"
            result_text += f")"
        
        self.add_result.setText(result_text)


class Win7Calculator(QMainWindow):
    """Windows 7 风格计算器主窗口（增强版）"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("计算器")
        self.setFixedSize(360, 520)
        
        # 计算器状态
        self.current_value = "0"
        self.previous_value = None
        self.current_operator = None
        self.waiting_for_operand = False
        self.memory = 0.0
        self.current_mode = "standard"  # standard, calendar, datecalc
        
        self.init_ui()
        
    def init_ui(self):
        """初始化界面"""
        # 主窗口样式 - Win7 玻璃效果背景
        self.setStyleSheet("""
            QMainWindow {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #E8F4FC, stop:0.5 #D6EBF5, stop:1 #C4E0F0);
            }
        """)
        
        # 中央部件
        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)
        layout.setSpacing(8)
        layout.setContentsMargins(12, 12, 12, 12)
        
        # 菜单栏模拟
        menu_bar = self.create_menu_bar()
        layout.addWidget(menu_bar)
        
        # 模式切换按钮
        mode_bar = self.create_mode_bar()
        layout.addWidget(mode_bar)
        
        # 堆叠窗口 - 切换不同模式
        self.stack = QStackedWidget()
        
        # 标准计算器页面
        self.calc_page = self.create_calc_page()
        self.stack.addWidget(self.calc_page)
        
        # 日历页面
        self.calendar_page = CalendarWithWeekWidget()
        self.stack.addWidget(self.calendar_page)
        
        # 日期计算页面
        self.datecalc_page = DateCalculatorWidget()
        self.stack.addWidget(self.datecalc_page)
        
        layout.addWidget(self.stack)
        
    def create_calc_page(self):
        """创建标准计算器页面"""
        page = QWidget()
        layout = QVBoxLayout(page)
        layout.setSpacing(8)
        layout.setContentsMargins(0, 0, 0, 0)
        
        # 显示屏
        self.display = self.create_display()
        layout.addWidget(self.display)
        
        # 内存指示器
        self.memory_label = QLabel("")
        self.memory_label.setFont(QFont("Segoe UI", 9))
        self.memory_label.setStyleSheet("color: #666; margin-left: 5px;")
        layout.addWidget(self.memory_label)
        
        # 按钮区域
        buttons_container = QWidget()
        buttons_layout = QVBoxLayout(buttons_container)
        buttons_layout.setSpacing(6)
        buttons_layout.setContentsMargins(0, 0, 0, 0)
        
        # 内存按钮行
        memory_row = self.create_memory_buttons()
        buttons_layout.addLayout(memory_row)
        
        # 主按钮网格
        main_grid = self.create_main_buttons()
        buttons_layout.addLayout(main_grid)
        
        layout.addWidget(buttons_container)
        layout.addStretch()
        
        return page
        
    def create_mode_bar(self):
        """创建模式切换栏"""
        mode_widget = QWidget()
        mode_layout = QHBoxLayout(mode_widget)
        mode_layout.setSpacing(5)
        mode_layout.setContentsMargins(0, 0, 0, 0)
        
        self.mode_buttons = {}
        
        modes = [
            ("🔢 标准", 0, "standard"),
            ("📅 日历", 1, "calendar"),
            ("📊 日期计算", 2, "datecalc"),
        ]
        
        for text, index, mode_id in modes:
            btn = Win7Button(text, is_special=(mode_id != "standard"))
            btn.setFixedHeight(32)
            btn.setCheckable(True)
            btn.clicked.connect(partial(self.switch_mode, index, mode_id))
            mode_layout.addWidget(btn)
            self.mode_buttons[mode_id] = btn
            
        # 默认选中标准模式
        self.mode_buttons["standard"].setChecked(True)
        self.mode_buttons["standard"].setStyleSheet(self.mode_buttons["standard"].styleSheet().replace(
            "stop:0 #F5F5F5", "stop:0 #CCE5FF"
        ).replace(
            "stop:1 #E0E0E0", "stop:1 #99CCFF"
        ))
        
        mode_layout.addStretch()
        return mode_widget
        
    def switch_mode(self, index, mode_id):
        """切换模式"""
        self.current_mode = mode_id
        self.stack.setCurrentIndex(index)
        
        # 更新按钮状态
        for mid, btn in self.mode_buttons.items():
            btn.setChecked(mid == mode_id)
            # 重置样式
            btn.update_style()
            if mid == mode_id:
                # 高亮选中状态
                btn.setStyleSheet(btn.styleSheet() + """
                    QPushButton {
                        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                            stop:0 #CCE5FF, stop:0.5 #99CCFF, stop:1 #66B2FF);
                        border: 2px solid #3399FF;
                    }
                """)
        
    def create_menu_bar(self):
        """创建菜单栏（Win7 风格）"""
        menu_widget = QWidget()
        menu_layout = QHBoxLayout(menu_widget)
        menu_layout.setSpacing(15)
        menu_layout.setContentsMargins(5, 2, 5, 2)
        
        # 查看菜单
        view_btn = QPushButton("查看(V)")
        view_btn.setFlat(True)
        view_btn.setFont(QFont("Segoe UI", 9))
        view_btn.setStyleSheet("""
            QPushButton {
                color: #333;
                padding: 2px 8px;
            }
            QPushButton:hover {
                background-color: #E5F3FF;
                border-radius: 2px;
            }
        """)
        view_btn.clicked.connect(self.toggle_mode)
        
        # 编辑菜单
        edit_btn = QPushButton("编辑(E)")
        edit_btn.setFlat(True)
        edit_btn.setFont(QFont("Segoe UI", 9))
        edit_btn.setStyleSheet("""
            QPushButton {
                color: #333;
                padding: 2px 8px;
            }
            QPushButton:hover {
                background-color: #E5F3FF;
                border-radius: 2px;
            }
        """)
        
        # 帮助菜单
        help_btn = QPushButton("帮助(H)")
        help_btn.setFlat(True)
        help_btn.setFont(QFont("Segoe UI", 9))
        help_btn.setStyleSheet("""
            QPushButton {
                color: #333;
                padding: 2px 8px;
            }
            QPushButton:hover {
                background-color: #E5F3FF;
                border-radius: 2px;
            }
        """)
        
        menu_layout.addWidget(view_btn)
        menu_layout.addWidget(edit_btn)
        menu_layout.addWidget(help_btn)
        menu_layout.addStretch()
        
        return menu_widget
        
    def create_display(self):
        """创建显示屏"""
        display = QLineEdit("0")
        display.setAlignment(Qt.AlignRight)
        display.setFont(QFont("Segoe UI", 24))
        display.setReadOnly(True)
        display.setFixedHeight(60)
        display.setStyleSheet("""
            QLineEdit {
                background-color: #F8F9FA;
                border: 2px solid #B8D4E3;
                border-radius: 4px;
                padding: 5px 10px;
                color: #333;
                selection-background-color: #3399FF;
            }
        """)
        return display
        
    def create_memory_buttons(self):
        """创建内存操作按钮"""
        layout = QHBoxLayout()
        layout.setSpacing(4)
        
        buttons = [
            ("MC", self.memory_clear),
            ("MR", self.memory_recall),
            ("MS", self.memory_store),
            ("M+", self.memory_add),
            ("M-", self.memory_subtract),
        ]
        
        for text, handler in buttons:
            btn = Win7Button(text, is_memory=True)
            btn.setFixedSize(52, 32)
            btn.clicked.connect(handler)
            layout.addWidget(btn)
            
        layout.addStretch()
        return layout
        
    def create_main_buttons(self):
        """创建主按钮网格"""
        grid = QGridLayout()
        grid.setSpacing(6)
        
        # 标准计算器布局
        standard_layout = [
            # 第一行
            [("←", self.backspace, True, False), ("CE", self.clear_entry, True, False), 
             ("C", self.clear, True, False), ("±", self.negate, True, False), 
             ("√", self.sqrt, True, False)],
            # 第二行
            [("7", partial(self.digit, "7"), False, False), ("8", partial(self.digit, "8"), False, False),
             ("9", partial(self.digit, "9"), False, False), ("/", partial(self.operator, "/"), True, True),
             ("%", self.percent, True, False)],
            # 第三行
            [("4", partial(self.digit, "4"), False, False), ("5", partial(self.digit, "5"), False, False),
             ("6", partial(self.digit, "6"), False, False), ("*", partial(self.operator, "*"), True, True),
             ("1/x", self.reciprocal, True, False)],
            # 第四行
            [("1", partial(self.digit, "1"), False, False), ("2", partial(self.digit, "2"), False, False),
             ("3", partial(self.digit, "3"), False, False), ("-", partial(self.operator, "-"), True, True),
             ("=", self.calculate, False, True)],
            # 第五行
            [("0", partial(self.digit, "0"), False, False, 2), (".", self.decimal, False, False),
             ("+", partial(self.operator, "+"), True, True)],
        ]
        
        row = 0
        for row_buttons in standard_layout:
            col = 0
            for btn_info in row_buttons:
                if len(btn_info) == 5:
                    text, handler, is_op, is_special, colspan = btn_info
                else:
                    text, handler, is_op, is_special = btn_info
                    colspan = 1
                    
                is_equal = (text == "=")
                btn = Win7Button(text, is_operator=is_op, is_special=is_special, is_equal=is_equal)
                
                if text == "0":
                    btn.setFixedSize(108, 40)
                else:
                    btn.setFixedSize(52, 40)
                    
                btn.clicked.connect(handler)
                grid.addWidget(btn, row, col, 1, colspan)
                col += colspan
            row += 1
            
        return grid
        
    # ==================== 计算功能 ====================
    
    def digit(self, num):
        """输入数字"""
        if self.waiting_for_operand:
            self.current_value = num
            self.waiting_for_operand = False
        else:
            if self.current_value == "0":
                self.current_value = num
            else:
                self.current_value += num
        self.update_display()
        
    def decimal(self):
        """输入小数点"""
        if self.waiting_for_operand:
            self.current_value = "0."
            self.waiting_for_operand = False
        elif "." not in self.current_value:
            self.current_value += "."
        self.update_display()
        
    def operator(self, op):
        """设置运算符"""
        if self.current_operator and not self.waiting_for_operand:
            self.calculate()
            
        self.previous_value = float(self.current_value)
        self.current_operator = op
        self.waiting_for_operand = True
        
    def calculate(self):
        """执行计算"""
        if self.current_operator is None or self.previous_value is None:
            return
            
        current = float(self.current_value)
        result = 0.0
        
        try:
            if self.current_operator == "+":
                result = self.previous_value + current
            elif self.current_operator == "-":
                result = self.previous_value - current
            elif self.current_operator == "*":
                result = self.previous_value * current
            elif self.current_operator == "/":
                if current == 0:
                    self.current_value = "除数不能为零"
                    self.update_display()
                    self.waiting_for_operand = True
                    return
                result = self.previous_value / current
                
            # 格式化结果
            if result == int(result):
                self.current_value = str(int(result))
            else:
                # 限制小数位数
                self.current_value = str(round(result, 10)).rstrip('0').rstrip('.')
                
            self.previous_value = result
            self.waiting_for_operand = True
            
        except Exception as e:
            self.current_value = "错误"
            
        self.update_display()
        
    def clear(self):
        """清空所有"""
        self.current_value = "0"
        self.previous_value = None
        self.current_operator = None
        self.waiting_for_operand = False
        self.update_display()
        
    def clear_entry(self):
        """清空当前输入"""
        self.current_value = "0"
        self.update_display()
        
    def backspace(self):
        """退格"""
        if len(self.current_value) > 1:
            self.current_value = self.current_value[:-1]
        else:
            self.current_value = "0"
        self.update_display()
        
    def negate(self):
        """取反"""
        if self.current_value != "0":
            if self.current_value.startswith("-"):
                self.current_value = self.current_value[1:]
            else:
                self.current_value = "-" + self.current_value
        self.update_display()
        
    def sqrt(self):
        """平方根"""
        try:
            val = float(self.current_value)
            if val < 0:
                self.current_value = "无效输入"
            else:
                result = math.sqrt(val)
                self.current_value = str(result).rstrip('0').rstrip('.')
            self.waiting_for_operand = True
        except:
            self.current_value = "错误"
        self.update_display()
        
    def percent(self):
        """百分比"""
        try:
            val = float(self.current_value)
            self.current_value = str(val / 100).rstrip('0').rstrip('.')
            self.waiting_for_operand = True
        except:
            self.current_value = "错误"
        self.update_display()
        
    def reciprocal(self):
        """倒数"""
        try:
            val = float(self.current_value)
            if val == 0:
                self.current_value = "除数不能为零"
            else:
                result = 1 / val
                self.current_value = str(result).rstrip('0').rstrip('.')
            self.waiting_for_operand = True
        except:
            self.current_value = "错误"
        self.update_display()
        
    # ==================== 内存功能 ====================
    
    def memory_clear(self):
        """MC - 清除内存"""
        self.memory = 0.0
        self.update_memory_label()
        
    def memory_recall(self):
        """MR - 调用内存"""
        self.current_value = str(self.memory).rstrip('0').rstrip('.')
        self.waiting_for_operand = True
        self.update_display()
        
    def memory_store(self):
        """MS - 存储到内存"""
        try:
            self.memory = float(self.current_value)
            self.update_memory_label()
            self.waiting_for_operand = True
        except:
            pass
            
    def memory_add(self):
        """M+ - 加到内存"""
        try:
            self.memory += float(self.current_value)
            self.update_memory_label()
            self.waiting_for_operand = True
        except:
            pass
            
    def memory_subtract(self):
        """M- - 从内存减去"""
        try:
            self.memory -= float(self.current_value)
            self.update_memory_label()
            self.waiting_for_operand = True
        except:
            pass
            
    def update_memory_label(self):
        """更新内存指示器"""
        if self.memory != 0:
            self.memory_label.setText("M")
        else:
            self.memory_label.setText("")
            
    # ==================== 辅助功能 ====================
    
    def update_display(self):
        """更新显示屏"""
        self.display.setText(self.current_value)
        
    def toggle_mode(self):
        """切换标准/科学模式（简化版）"""
        # 可扩展为完整科学计算器
        pass
        
    def keyPressEvent(self, event):
        """键盘支持"""
        key = event.key()
        text = event.text()
        
        # 数字键
        if text.isdigit():
            self.digit(text)
        # 运算符
        elif text in "+-*/":
            self.operator(text)
        elif key == Qt.Key_Return or key == Qt.Key_Enter or text == "=":
            self.calculate()
        elif key == Qt.Key_Backspace:
            self.backspace()
        elif key == Qt.Key_Escape:
            self.clear()
        elif text == ".":
            self.decimal()
            
            
def main():
    """主函数"""
    app = QApplication(sys.argv)
    app.setStyle("Windows")  # 使用 Windows 风格
    
    # 设置应用字体
    font = QFont("Segoe UI", 9)
    if not QFont(font).exactMatch():
        font = QFont("Microsoft YaHei", 9)
    app.setFont(font)
    
    calculator = Win7Calculator()
    calculator.show()
    
    sys.exit(app.exec_())
    
    
if __name__ == "__main__":
    main()
