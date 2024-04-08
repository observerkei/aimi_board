import sys
import subprocess


_audio_driver_prefix = "audio_driver"


class RunCodeReturn:
    returncode: int = -1
    stdout: str = ""
    stderr: str = ""


def make_ret(result: None):
    run = RunCodeReturn()
    run.stdout = str(result.stdout.decode("utf-8"))
    run.stderr = str(result.stderr.decode("utf-8"))
    run.returncode = int(result.returncode)
    return run


def run_cmd(cmd: list[str]):
    return subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def chat_to_audio(chat: str, audio: str):
    """
    将聊天内容转换为音频文件。

    Args:
        chat (str): 要转换的聊天内容。
        audio (str): 输出的音频文件路径。
    """
    
    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/chat_to_audio.sh", audio, chat])
    return make_ret(r)


def audio_to_speak(audio: str):
    """
    将音频文件播放出来。

    Args:
        audio (str): 要播放的音频文件路径。
    """

    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/audio_to_speak.sh", audio])
    return make_ret(r)


def chat_to_speak(chat: str):
    """
    将聊天内容直接转换为语音播放。

    Args:
        chat (str): 要转换并播放的聊天内容。
    """

    rc = chat_to_audio(chat, "/tmp/audio.mp3")
    ra = audio_to_speak("/tmp/audio.mp3")

    rc.returncode += ra.returncode
    rc.stdout = f"{rc.stdout}\n{ra.stdout}"
    rc.stderr = f"{rc.stderr}\n{ra.stderr}"
    return rc


def record(device: str, filename: str):
    """
    录制音频到指定文件。

    Args:
        device (str): 音频设备名称或ID。
        filename (str): 输出的音频文件路径。
    """

    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/record.sh", device, filename])
    return make_ret(r)


def splicing_audio(files: list[str], outfile: str):
    """
    拼接多个音频文件成一个音频文件。

    Args:
        files (list[str]): 要拼接的音频文件列表。
        outfile (str): 输出的音频文件路径。
    """
    
    run_cmd(["rm", "-f ", outfile])

    cmd = ["/bin/bash", f"{_audio_driver_prefix}/splicing_audio.sh"]
    cmd.extend(files)
    cmd.append(outfile)

    r = run_cmd(cmd)
    return make_ret(r)
