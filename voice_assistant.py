from display_driver import Display, View, Color
from button_driver import Button, ButtonType
from audio_driver import record, chat_to_speak, splicing_audio
from openai_api import OpenAIAPI
from azure_api import voice_recognition
import threading
import time

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
    preset: str = "你是个AI助手, 请尽可能简短的回复. "

    def __init__(self):
        self.__display_init()
        self.__buttun_init()
        self.__bot_init()
    
    def __bot_init(self):
        self.bot = OpenAIAPI()

    def __display_init(self):
        self.display = Display("./display_driver/display.so", "/dev/fb0", "./display_driver/font/")
        width = self.display.display_get_width()
        height = self.display.display_get_height()
        log_dbg(f"width: {width}, height: {height}")

        self.uv = UserView(width, height)
        self.av = AssistantView(width, height)

        self.display.display_view_print(self.uv, "UTF-8", "USER: 1+1=?")
        self.display.display_view_print(self.av, "UTF-8", "AI: 1+1=2")
        self.display.display_fflush()


    def __buttun_init(self):
        self.button = Button('/dev/input/event0')  
        buttun_thread = threading.Thread(target=self.button.server)
        buttun_thread.setDaemon(True)  # 将线程设置为守护线程
        buttun_thread.start()  # 启动按键监听线程
    
    def run(self):
        audio_records = []

        log_dbg("start listen. ")

        while True:
            if self.button.is_key_pressed(ButtonType.KEY_RIGHT):
                log_dbg(f"record..")
                
                Listening = "Listening"
                for _ in audio_records:
                    Listening += "."
                self.display.display_view_clear(self.uv)
                self.display.display_view_print(self.uv, "UTF-8", f"USER: ({Listening})")
                self.display.display_fflush()

                filename = f"/tmp/record/{len(audio_records)}.wav"
                ret = record(filename)
                if ret.returncode != 0:
                    log_dbg(f"record: {ret.stderr}")
                else:
                    log_dbg(f"record err: {ret.stderr}")
                audio_records.append(filename)

            elif len(audio_records) and not self.button.is_key_pressed(ButtonType.KEY_RIGHT):
                log_dbg(f"splicing audio..")
                self.display.display_view_clear(self.uv)
                self.display.display_view_print(self.uv, "UTF-8", f"USER: (splicing audio...)")
                self.display.display_fflush()

                filename = splicing_audio(audio_records)
                log_dbg(f"splicing audio: {audio_records}")
                audio_records = []

                self.display.display_view_clear(self.uv)
                self.display.display_view_print(self.uv, "UTF-8", f"USER: (voice recognition...)")
                self.display.display_fflush()

                chat = voice_recognition(filename)
                self.display.display_view_clear(self.uv)
                self.display.display_view_print(self.uv, "UTF-8", f"USER: {chat}")
                self.display.display_fflush()
                log_dbg(f"voice_recognition: {chat}")

                self.display.display_view_clear(self.av)
                self.display.display_view_print(self.av, "UTF-8", f"AI: ")
                self.display.display_fflush()

                chunk = ""
                prev_text = ""
                for res in self.bot.ask(question=chat, preset=self.preset):
                    if res["message"] == -1:
                        continue
                    log_dbg(f"res: {prev_text}")

                    chunk = res["message"][len(prev_text) :]
                    self.display.display_view_print(self.av, "UTF-8", chunk)
                    self.display.display_fflush()

                    prev_text = res["message"]

                if len(prev_text):
                    ret = chat_to_speak(prev_text)
                    if ret.returncode == 0:
                        log_dbg(f"speak: {ret.stdout}")
                    else:
                        log_dbg(f"speak err: {ret.stderr}")
            else:
                time.sleep(0.1)


if __name__ == '__main__':
    VoiceAssistant().run()