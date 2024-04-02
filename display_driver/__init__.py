from ctypes import Structure, cdll, c_int, c_size_t, c_uint16, c_void_p, c_char, POINTER
from typing import Any


class Color:
    Black: int = c_uint16(0x0000)
    While: int = c_uint16(0xFFFF)
    Grey: int = c_uint16(0xE73C)

    def create(color: int):
        return c_uint16(color)


#
# Structure link: https://docs.python.org/3/library/ctypes.html
# typedef struct view_t {
#     const size_t start_x;
#     const size_t start_y;
#     const size_t width;
#     const size_t height;
#     size_t now_x;
#     size_t now_y;
#     framebuffer_color_t font_color;
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
        self.display_so.display_fflush(self.display_driver)

    def display_view_clear(self, v: View):
        self.display_so.display_view_clear(self.display_driver, v)

    def display_set_debug(self, enable: int):
        self.display_driver.display_set_debug(enable)

    def display_get_width(self):
        return self.display_so.display_get_width(self.display_driver)

    def display_get_height(self):
        return self.display_so.display_get_height(self.display_driver)

    def display_set_cache_color(self, x: int, y: int, color: int):
        self.display_so.display_set_cache_color(self.display_driver, x, y, color)

    def display_view_print(self, v: View, from_code: str, content: str):
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
