from ctypes import Structure, cdll, c_int, c_size_t, c_uint16, c_void_p, c_char, POINTER
from typing import Any


class Color:
    Black: int = c_uint16(0x0000)
    While: int = c_uint16(0xFFFF)
    Grey: int = c_uint16(0xE73C)
    Yellow: int = c_uint16(0xf7ce)
    Green: int = c_uint16(0x07c0)
    Blue: int = c_uint16(0x001e)

    def create(color: int):
        return c_uint16(color)


# 
#     @ Display
#     @ 原点点定义: 0点为屏幕正方的左上角
#     @ 方向定义
#  
#         |<--width-->
#     —— 0+----------> x++
#     ^   |
#     |   |
#  height |
#     |   |
#     v   v
#         y++
# 
#
# Structure link: https://docs.python.org/3/library/ctypes.html
# typedef struct view_t {
#     const size_t start_x;            // 视图开始x位置
#     const size_t start_y;            // 视图开始y位置
#     const size_t width;              // 视图宽
#     const size_t height;             // 视图高
#     size_t now_x;                    // 当前绘制x位置
#     size_t now_y;                    // 当前绘制y位置
#     framebuffer_color_t font_color;  // 绘制颜色
# } view_t;

class View(Structure):
    start_x: int
    start_y: int
    width: int
    height: int
    now_x: int
    now_y: int
    font_color: int

    _fields_ = [
        ("start_x", c_size_t),
        ("start_y", c_size_t),
        ("width", c_size_t),
        ("height", c_size_t),
        ("now_x", c_size_t),
        ("now_y", c_size_t),
        ("font_color", c_uint16),
    ]


class Display:
    display_so: Any
    display_driver: Any = None

    def __init__(self, driver_so_path: str, framebuffer_dev: str, font_path: str):
        self.display_so = cdll.LoadLibrary(driver_so_path)
        # 传入默认的参数初始化
        self.__hook_setup()
        self.display_driver = self.display_so.display_init(framebuffer_dev.encode(), font_path.encode())

    def __hook_setup(self):
        # void display_exit(display_t *d);
        self.display_so.display_exit.argtypes = [POINTER(c_void_p)]

        # display_t *display_init(const char *fb_dev, const char *font_path);
        self.display_so.display_init.argtypes = [POINTER(c_char), POINTER(c_char)]
        self.display_so.display_init.restype = POINTER(c_void_p)

        # void display_fflush(display_t *d);
        self.display_so.display_fflush.argtypes = [POINTER(c_void_p)]

        # void display_view_clear(display_t* d, view_t* v);
        self.display_so.display_view_clear.argtypes = [POINTER(c_void_p), POINTER(View)]

        # void display_set_debug(char enable);
        self.display_so.display_set_debug.argtypes = [c_char]

        # size_t display_get_width(display_t *d);
        self.display_so.display_get_width.argtypes = [POINTER(c_void_p)]
        self.display_so.display_get_width.restype = c_size_t

        # size_t display_get_height(display_t *d);
        self.display_so.display_get_height.argtypes = [POINTER(c_void_p)]
        self.display_so.display_get_height.restype = c_size_t

        # void display_set_cache_color(display_t* d, size_t x, size_t y, framebuffer_color_t color);
        self.display_so.display_set_cache_color.argtypes = [
            POINTER(c_void_p),
            c_size_t,
            c_size_t,
            c_uint16,
        ]

        # int display_view_print(display_t* d, view_t *v, const char *from_code, const char* str, size_t str_len);
        self.display_so.display_view_print.argtypes = [
            POINTER(c_void_p),
            POINTER(View),
            POINTER(c_char),
            POINTER(c_char),
            c_size_t,
        ]
        self.display_so.display_view_print.restype = c_int

    def display_fflush(self):
        """
        刷新显示设备的内容。
        """

        self.display_so.display_fflush(self.display_driver)

    def display_view_clear(self, v: View):
        """
        清空指定视图的内容。

        Args:
            v (View): 要清空内容的视图对象。
        """

        self.display_so.display_view_clear(self.display_driver, v)

    def display_set_debug(self, enable: int):
        """
        设置调试模式。

        Args:
            enable (int): 是否启用调试模式, 1 为启用, 0 为禁用。
        """

        self.display_driver.display_set_debug(enable)

    def display_get_width(self):
        """
        获取显示设备的宽度。

        Returns:
            int: 显示设备的宽度。
        """

        return self.display_so.display_get_width(self.display_driver)

    def display_get_height(self):
        """
        获取显示设备的高度。

        Returns:
            int: 显示设备的高度。
        """

        return self.display_so.display_get_height(self.display_driver)

    def display_set_cache_color(self, x: int, y: int, color: int):
        """
        在缓存上设置指定位置的颜色。

        Args:
            x (int): 指定位置的横坐标。
            y (int): 指定位置的纵坐标。
            color (int): 要设置的颜色。
        """

        self.display_so.display_set_cache_color(self.display_driver, x, y, color)

    def display_view_print(self, v: View, from_code: str, content: str):
        """
        在指定视图上打印字符串。

        Args:
            v (View): 要打印字符串的视图对象。
            from_code (str): 字符串的编码格式。
            content (str): 要打印的字符串内容。
        """

        return self.display_so.display_view_print(
            self.display_driver,
            v,
            from_code.encode(),
            content.encode(),
            len(content.encode()),
        )

    def __del__(self):
        if self.display_driver:
            self.display_so.display_exit(self.display_driver)
