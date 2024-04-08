import evdev
import time

log_dbg = print

class ButtonType:
    KEY_RIGHT = "KEY_RIGHT"
    KEY_LEFT = "KEY_LEFT"
    KEY_B = "KEY_B"
    KEY_A = "KEY_A"

class Button:
    running: bool = False
    device_path: str # 键盘设备路径
    device = None
    key_pressed_times: dict = {}

    def __init__(self, device_path = '/dev/input/event0'):
        self.device_path = device_path

    # 开始监听按键事件
    def server(self):
        """
        按键监听服务器
        """

        try:
            self.device = evdev.InputDevice(self.device_path)
            self.running = True
            log_dbg("开始监听按键事件...")
            log_dbg("设备名称:", self.device.name)
            log_dbg("设备路径:", self.device.path)
            log_dbg("设备信息:", self.device)

            for event in self.device.read_loop():
                if event.type == evdev.ecodes.EV_KEY:
                    key_event = evdev.categorize(event)
                    if key_event.keystate == key_event.key_down:
                        log_dbg(f"按键按下: {key_event.keycode}")
                        self.key_pressed_times[key_event.keycode] = time.time()  # 记录按键按下时间戳
                    elif key_event.keystate == key_event.key_up:
                        log_dbg(f"按键松开: {key_event.keycode}")
                        if key_event.keycode in self.key_pressed_times:
                            key_down_duration = time.time() - self.key_pressed_times[key_event.keycode]
                            log_dbg(f"按键按下时长: {key_down_duration:.2f} 秒")
                            del self.key_pressed_times[key_event.keycode]  # 删除按键的按下时间戳
        except Exception as e:
            log_dbg(f"发生错误: {e}")
        finally:
            if self.device:
                self.device.close()
                self.device = None

    # 返回按键是否按下的检测结果
    def is_key_pressed(self, keycode):
        """
        返回按键是否按下的检测结果

        Args:
            keycode (str): 要检测的按键名称
        
        Return:
             (bool): 按键是否按下, 按下 True 未按下 False
        """

        return keycode in self.key_pressed_times

# 示例用法
if __name__ == "__main__":
    import threading

    button = Button()  

    thread = threading.Thread(target=button.server)
    thread.setDaemon(True)  # 将线程设置为守护线程
    thread.start()  # 启动按键监听线程

    time.sleep(5)  # 假设等待5秒钟
    if button.is_key_pressed(evdev.ecodes.KEY_ENTER):
        log_dbg(f"{evdev.ecodes.KEY_ENTER}键被按下")
    else:
        log_dbg(f"{evdev.ecodes.KEY_ENTER}键未被按下")

    thread.join()
