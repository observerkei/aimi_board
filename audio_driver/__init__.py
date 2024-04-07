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


def chat_to_speak(chat: str):
    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/chat_to_speak.sh", chat])
    return make_ret(r)


def record(filename: str):
    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/record.sh", filename])
    return make_ret(r)


def splicing_audio(files: list[str]):
    cmd = ["/bin/bash", f"{_audio_driver_prefix}/splicing_audio.sh"]
    cmd.extend(files)
    r = run_cmd(cmd)
    ret = make_ret(r)

    if ret.returncode == 0:
        print(f"splicing_audio: {ret.stdout}")
        return "/tmp/record/record.wav"
    
    print(f"splicing_audio err: {ret.stderr}")
    return ""
