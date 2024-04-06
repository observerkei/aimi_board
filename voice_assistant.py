from display_driver import Display, View, Color
from button_driver import Button, ButtonType
import threading

log_dbg = print


class AssistantView(View):
    def __init__(self, driver_width: int, driver_height: int):
        super().__init__(
            start_x = 0,
            start_y = 0,
            width = driver_width,
            height = int(11 * driver_height / 15),
            now_x = 0,
            now_y = 0,
            font_color = Color.While,
        )

class UserView(View):
    def __init__(self, driver_width: int, driver_height: int):
        super().__init__(
            start_x = 0,
            start_y = int(11 * driver_height / 15),
            width = driver_width,
            height = int(4 * driver_height / 15),
            now_x = 0,
            now_y = int(11 * driver_height / 15),
            font_color = Color.While,
        )


class VoiceAssistant:
    
    def __init__(self):
        self.__display_init()
        self.__buttun_init()

    def __display_init(self):
        self.display = Display("./display_driver/display.so", "/dev/fb0", "./display_driver/font/")
        width = self.display.display_get_width()
        height = self.display.display_get_height()
        log_dbg(f"width: {width}, height: {height}")

        self.uv = UserView(width, height)
        self.av = AssistantView(width, height)

        v = View(width = width, height = height, font_color = Color.While)

        log_dbg(f"v.font_color: {v.font_color}")

        self.display.display_view_print(v, "UTF-8", "============TEST")

        self.display.display_view_print(self.uv, "UTF-8", "ABC")
        self.display.display_view_print(self.av, "UTF-8", "123")
        self.display.display_fflush()


    def __buttun_init(self):
        self.button = Button('/dev/input/event0')  
        buttun_thread = threading.Thread(target=self.button.server)
        buttun_thread.setDaemon(True)  # 将线程设置为守护线程
        buttun_thread.start()  # 启动按键监听线程

    def run(self):
        pass
