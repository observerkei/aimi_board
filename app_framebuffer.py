from display_driver import Display, View, Color

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

display = Display("./display_driver/display.so", "/dev/fb0", "./display_driver/font/")

width = display.display_get_width()
height = display.display_get_height()

print(f"width: {width}, height: {height}")

uv = UserView(width, height)
av = AssistantView(width, height)

v = View(width = width, height = height, font_color = Color.While)

print(f"v.font_color: {v.font_color}")

display.display_view_print(v, "UTF-8", "============TEST")

display.display_view_print(uv, "UTF-8", "ABC")
display.display_view_print(av, "UTF-8", "123")
display.display_fflush()
