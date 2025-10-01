import socket

def send_command(ip, command):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((ip, 3333))
            s.sendall(command.encode())
            # 可选：接收响应
            # response = s.recv(1024)
            # print("Response:", response.decode())
    except Exception as e:
        print("Error:", e)

# 使用示例
send_command("192.168.1.101", "gfdh")