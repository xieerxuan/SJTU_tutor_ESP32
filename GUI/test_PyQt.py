import sys
import socket
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QLabel, QLineEdit, QPushButton, 
                             QGroupBox, QTextEdit, QGridLayout, QFrame)
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QTimer, QPoint, QRectF
from PyQt5.QtGui import QFont, QColor, QPainter, QPen, QPolygon, QBrush, QPainterPath
import math

class Speedometer(QWidget):
    """自定义速度仪表盘"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(300, 300)
        self.value = 0
        self.max_value = 600  # 最大速度值
        self.animation_timer = QTimer(self)
        self.animation_timer.timeout.connect(self.update_value_animation)
        self.target_value = 0
        self.animation_speed = 50  # 动画速度，值越大越快
        
    def set_speed(self, speed):
        """设置速度值并启动动画"""
        self.target_value = min(max(speed, 0), self.max_value)
        if not self.animation_timer.isActive():
            self.animation_timer.start(20)  # 每20ms更新一次
        
    def update_value_animation(self):
        """更新值的动画效果"""
        if abs(self.value - self.target_value) < self.animation_speed:
            self.value = self.target_value
            self.animation_timer.stop()
        else:
            if self.value < self.target_value:
                self.value += self.animation_speed
            else:
                self.value -= self.animation_speed
        self.update()  # 触发重绘
        
    def paintEvent(self, event):
        """绘制仪表盘"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        
        # 计算中心点和半径
        center = QPoint(self.width() // 2, self.height() // 2)
        radius = min(self.width(), self.height()) // 2 - 10
        
        # 绘制外圆
        painter.setPen(QPen(QColor(100, 100, 100), 3))
        painter.setBrush(QColor(240, 240, 240))
        painter.drawEllipse(center, radius, radius)
        
        # 绘制刻度
        painter.save()
        painter.translate(center.x(), center.y())
        
        # 绘制主要刻度
        for i in range(0, 11):
            angle = 225 - i * 270 / 10  # 从225度到-45度
            rad = math.radians(angle)
            
            # 刻度线
            x1 = (radius - 15) * math.cos(rad)
            y1 = -(radius - 15) * math.sin(rad)
            x2 = (radius - 5) * math.cos(rad)
            y2 = -(radius - 5) * math.sin(rad)
            
            painter.setPen(QPen(QColor(0, 0, 0), 2))
            painter.drawLine(int(x1), int(y1), int(x2), int(y2))
            
            # 刻度值
            value = i * self.max_value / 10
            text_x = (radius - 25) * math.cos(rad)
            text_y = -(radius - 25) * math.sin(rad)
            
            painter.setPen(QColor(0, 0, 0))
            painter.drawText(QRectF(text_x - 15, text_y - 10, 30, 20), 
                            Qt.AlignCenter, str(int(value)))
        
        # 绘制次要刻度
        for i in range(0, 51):
            angle = 225 - i * 270 / 50
            rad = math.radians(angle)
            
            x1 = (radius - 10) * math.cos(rad)
            y1 = -(radius - 10) * math.sin(rad)
            x2 = (radius - 5) * math.cos(rad)
            y2 = -(radius - 5) * math.sin(rad)
            
            painter.setPen(QPen(QColor(0, 0, 0), 1))
            painter.drawLine(int(x1), int(y1), int(x2), int(y2))
        
        painter.restore()
        
        # 绘制指针
        angle = 225 + (self.value / self.max_value) * 270
        rad = math.radians(angle)
        
        painter.save()
        painter.translate(center.x(), center.y())
        painter.rotate(angle)
        
        # 指针三角形
        pointer = QPolygon([
            QPoint(0, 0),
            QPoint(-5, -10),
            QPoint(0, -radius + 15),
            QPoint(5, -10)
        ])
        
        painter.setPen(QPen(QColor(255, 0, 0), 2))
        painter.setBrush(QBrush(QColor(255, 0, 0)))
        painter.drawPolygon(pointer)
        
        painter.restore()
        
        # 绘制中心圆
        painter.setPen(QPen(QColor(100, 100, 100), 2))
        painter.setBrush(QBrush(QColor(200, 200, 200)))
        painter.drawEllipse(center, 10, 10)
        
        # 绘制当前值
        painter.setPen(QColor(0, 0, 0))
        painter.setFont(QFont("Arial", 12, QFont.Bold))
        painter.drawText(QRectF(center.x() - 50, center.y() + radius - 40, 100, 30), 
                        Qt.AlignCenter, f"{self.value} RPM")

class TCPClient(QThread):
    """TCP客户端线程，用于处理与ESP32的通信"""
    data_received = pyqtSignal(str)
    connection_status = pyqtSignal(bool, str)
    
    def __init__(self):
        super().__init__()
        self.sock = None
        self.host = None
        self.port = None
        self.connected = False
        self.running = True
        
    def set_server(self, host, port):
        """设置服务器地址和端口"""
        self.host = host
        self.port = port
        
    def connect(self):
        """连接到服务器"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(2)  # 设置超时时间为2秒
            self.sock.connect((self.host, self.port))
            self.connected = True
            self.connection_status.emit(True, f"Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            self.connected = False
            self.connection_status.emit(False, f"Connection failed: {str(e)}")
            return False
            
    def disconnect(self):
        """断开连接"""
        self.connected = False
        self.running = False
        if self.sock:
            self.sock.close()
            
    def send_command(self, command):
        """发送命令到服务器"""
        if self.connected and self.sock:
            try:
                self.sock.sendall(command.encode('utf-8'))
                return True
            except Exception as e:
                self.connected = False
                self.connection_status.emit(False, f"Send failed: {str(e)}")
                return False
        return False
        
    def run(self):
        """线程主循环，接收数据"""
        while self.running:
            if self.connected and self.sock:
                try:
                    data = self.sock.recv(1024)
                    if data:
                        self.data_received.emit(data.decode('utf-8'))
                    else:
                        self.connected = False
                        self.connection_status.emit(False, "Connection closed by server")
                except socket.timeout:
                    continue  # 超时是正常的，继续等待
                except Exception as e:
                    self.connected = False
                    self.connection_status.emit(False, f"Receive error: {str(e)}")
            else:
                # 如果没有连接，等待一段时间再检查
                self.msleep(100)  # 使用QThread的msleep方法
                
        if self.sock:
            self.sock.close()

class MainWindow(QMainWindow):
    """主窗口"""
    def __init__(self):
        super().__init__()
        self.tcp_client = TCPClient()
        self.init_ui()
        self.init_tcp_client()
        
    def init_ui(self):
        """初始化用户界面"""
        self.setWindowTitle("ESP32 WiFi Controller")
        self.setGeometry(100, 100, 900, 600)
        
        # 创建中央部件
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # 主布局
        main_layout = QHBoxLayout()
        central_widget.setLayout(main_layout)
        
        # 左侧控制面板
        control_panel = QGroupBox("Control Panel")
        control_layout = QVBoxLayout()
        
        # 连接设置
        connection_group = QGroupBox("Connection Settings")
        connection_layout = QGridLayout()
        
        connection_layout.addWidget(QLabel("IP Address:"), 0, 0)
        self.ip_edit = QLineEdit("192.168.1.116") 
        connection_layout.addWidget(self.ip_edit, 0, 1)
        
        connection_layout.addWidget(QLabel("Port:"), 1, 0)
        self.port_edit = QLineEdit("3333")
        connection_layout.addWidget(self.port_edit, 1, 1)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.clicked.connect(self.toggle_connection)
        connection_layout.addWidget(self.connect_btn, 2, 0, 1, 2)
        
        connection_group.setLayout(connection_layout)
        control_layout.addWidget(connection_group)
        
        # 命令按钮
        commands_group = QGroupBox("Motor Control")
        commands_layout = QVBoxLayout()
        
        self.start_btn = QPushButton("START")
        self.start_btn.clicked.connect(lambda: self.send_command("START"))
        commands_layout.addWidget(self.start_btn)
        
        self.stop_btn = QPushButton("STOP")
        self.stop_btn.clicked.connect(lambda: self.send_command("STOP"))
        commands_layout.addWidget(self.stop_btn)
        
        commands_group.setLayout(commands_layout)
        control_layout.addWidget(commands_group)
        
        # 状态信息
        status_group = QGroupBox("Status")
        status_layout = QVBoxLayout()
        
        self.status_text = QTextEdit()
        self.status_text.setReadOnly(True)
        self.status_text.setMaximumHeight(150)
        status_layout.addWidget(self.status_text)
        
        status_group.setLayout(status_layout)
        control_layout.addWidget(status_group)
        
        control_panel.setLayout(control_layout)
        main_layout.addWidget(control_panel, 1)
        
        # 右侧仪表盘
        dashboard_panel = QGroupBox("Dashboard")
        dashboard_layout = QVBoxLayout()
        
        # 速度仪表盘
        self.speedometer = Speedometer()
        dashboard_layout.addWidget(self.speedometer)
        
        # 速度值显示
        self.speed_label = QLabel("0 RPM")
        self.speed_label.setAlignment(Qt.AlignCenter)
        self.speed_label.setFont(QFont("Arial", 16, QFont.Bold))
        dashboard_layout.addWidget(self.speed_label)
        
        dashboard_panel.setLayout(dashboard_layout)
        main_layout.addWidget(dashboard_panel, 2)
        
        # 禁用按钮直到连接建立
        self.set_buttons_enabled(False)
        
    def init_tcp_client(self):
        """初始化TCP客户端"""
        self.tcp_client.data_received.connect(self.handle_data_received)
        self.tcp_client.connection_status.connect(self.handle_connection_status)
        self.tcp_client.start()
        
    def toggle_connection(self):
        """切换连接状态"""
        if self.tcp_client.connected:
            self.tcp_client.disconnect()
            self.connect_btn.setText("Connect")
        else:
            host = self.ip_edit.text()
            try:
                port = int(self.port_edit.text())
                self.tcp_client.set_server(host, port)
                if self.tcp_client.connect():
                    self.connect_btn.setText("Disconnect")
            except ValueError:
                self.status_text.append("Invalid port number")
                
    def send_command(self, command):
        """发送命令到ESP32"""
        if self.tcp_client.connected:
            self.tcp_client.send_command(command)
            self.status_text.append(f"Sent: {command}")
            
    def handle_data_received(self, data):
        """处理接收到的数据"""
        self.status_text.append(f"Received: {data}")
        
        # 解析速度数据
        if data.startswith("Current speed:"):
            try:
                speed_str = data.split(":")[1].strip()
                speed = int(speed_str)
                self.update_speed(speed)
            except ValueError:
                self.status_text.append(f"Error parsing speed data: {data}")
                
    def handle_connection_status(self, connected, message):
        """处理连接状态变化"""
        self.status_text.append(message)
        self.set_buttons_enabled(connected)
        
    def update_speed(self, speed):
        """更新速度显示"""
        self.speedometer.set_speed(speed)
        self.speed_label.setText(f"{speed} RPM")
        
    def set_buttons_enabled(self, enabled):
        """设置按钮启用状态"""
        self.start_btn.setEnabled(enabled)
        self.stop_btn.setEnabled(enabled)
        
    def closeEvent(self, event):
        """应用程序关闭事件"""
        self.tcp_client.disconnect()
        self.tcp_client.wait()  # 等待线程结束
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())