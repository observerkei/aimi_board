from display_driver import Display, View, Color
from button_driver import Button, ButtonType
from audio_driver import record, chat_to_audio, audio_to_speak, splicing_audio
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
    preset: str = "你是个AI助手, 请尽可能简短的回复. 如我问: `你好`, 你回答: `我很好, 你呢?` "

    def __init__(self):
        self.__display_init()
        self.__record_init()
        self.__buttun_init()
        self.__bot_init()
    
    def __bot_init(self):
        self.bot = OpenAIAPI()

    def __record_init(self):
        self.recoed_device = "hw:0,0"

    def __display_init(self):
        self.display = Display("./display_driver/display.so", "/dev/fb0", "./display_driver/font/")
        width = self.display.display_get_width()
        height = self.display.display_get_height()
        log_dbg(f"width: {width}, height: {height}")

        self.uv = UserView(width, height)
        self.av = AssistantView(width, height)

        self.display.display_view_print(self.uv, "UTF-8", "USER: 1+1=? \n(Press Right-button to start)")
        self.display.display_view_print(self.av, "UTF-8", "AI: 1+1=2")
        self.display.display_fflush()


    def __buttun_init(self):
        self.button = Button('/dev/input/event0')  
        buttun_thread = threading.Thread(target=self.button.server)
        buttun_thread.setDaemon(True)  # 将线程设置为守护线程
        buttun_thread.start()  # 启动按键监听线程
    
    def speak_chat(self, chat):
        self.display.display_view_clear(self.av)
        self.display.display_view_print(self.av, "UTF-8", f"AI: {chat}\n(conceive a sound...)")
        self.display.display_fflush()
        
        ret = chat_to_audio(chat, "/tmp/audio.mp3")
        if ret.returncode != 0:
            log_dbg(f"speak err: {ret.stderr}")
            self.display.display_view_clear(self.av)
            self.display.display_view_print(self.av, "UTF-8", f"AI: {chat}\n(speak err...)")
            self.display.display_fflush()
            return
        
        
        self.display.display_view_clear(self.av)
        self.display.display_view_print(self.av, "UTF-8", f"AI: {chat}\n(try to speak...)")
        self.display.display_fflush()
        
        ret = audio_to_speak("/tmp/audio.mp3")
        if ret.returncode != 0:
            log_dbg(f"speak err: {ret.stderr}")
            self.display.display_view_clear(self.av)
            self.display.display_view_print(self.av, "UTF-8", f"AI: {chat}\n(speak err...)")
            self.display.display_fflush()
            return
        
        log_dbg(f"speak: {ret.stdout}")
        self.display.display_view_clear(self.av)
        self.display.display_view_print(self.av, "UTF-8", f"AI: {chat}")
        self.display.display_fflush()

    def voices_to_chat(self, audio_records):
        log_dbg(f"splicing audio..")
        self.display.display_view_clear(self.uv)
        self.display.display_view_print(self.uv, "UTF-8", f"USER: (splicing audio...)")
        self.display.display_fflush()

        save_record = "/tmp/record/recoed.wav"
        ret = splicing_audio(audio_records, save_record)
        if ret.returncode != 0:
            log_dbg(f"splicing_audio: {ret.stderr}")
        else:
            log_dbg(f"splicing_audio err: {ret.stderr}")

        self.display.display_view_clear(self.uv)
        self.display.display_view_print(self.uv, "UTF-8", f"USER: (voice recognition...)")
        self.display.display_fflush()

        chat = voice_recognition(save_record)
        self.display.display_view_clear(self.uv)
        self.display.display_view_print(self.uv, "UTF-8", f"USER: {chat}")
        self.display.display_fflush()

        return chat
    
    def recording_voice(self, idx: int):
        log_dbg(f"record..")
        
        Listening = "Listening"
        for _ in range(0, idx):
            Listening += "."
        self.display.display_view_clear(self.uv)
        self.display.display_view_print(self.uv, "UTF-8", f"USER: ({Listening})")
        self.display.display_fflush()

        filename = f"/tmp/record/{idx}.wav"
        ret = record(self.recoed_device, filename)
        if ret.returncode != 0:
            log_dbg(f"record err: {ret.stderr}")
            return None
        else:
            log_dbg(f"record: {ret.stdout}")
        
        return filename

    def run(self):
        audio_records = []

        log_dbg("start listen. ")

        while True:
            if self.button.is_key_pressed(ButtonType.KEY_RIGHT):
                filename = self.recording_voice(len(audio_records))
                if not filename:
                    continue
                audio_records.append(filename)

            elif len(audio_records) and not self.button.is_key_pressed(ButtonType.KEY_RIGHT):
                chat = self.voices_to_chat(audio_records)
                audio_records = []
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
                    self.speak_chat(prev_text)
                    
            else:
                time.sleep(0.1)


if __name__ == '__main__':
    VoiceAssistant().run()