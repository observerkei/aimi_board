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
    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/chat_to_audio.sh", audio, chat])
    return make_ret(r)


def audio_to_speak(audio: str):
    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/audio_to_speak.sh", audio])
    return make_ret(r)


def chat_to_speak(chat: str):
    rc = chat_to_audio(chat, "/tmp/audio.mp3")
    ra = audio_to_speak("/tmp/audio.mp3")

    rc.returncode += ra.returncode
    rc.stdout = f"{rc.stdout}\n{ra.stdout}"
    rc.stderr = f"{rc.stderr}\n{ra.stderr}"
    return rc


def record(device: str, filename: str):
    r = run_cmd(["/bin/bash", f"{_audio_driver_prefix}/record.sh", device, filename])
    return make_ret(r)


def splicing_audio(files: list[str], outfile: str):
    run_cmd(["rm", "-f ", outfile])

    cmd = ["/bin/bash", f"{_audio_driver_prefix}/splicing_audio.sh"]
    cmd.extend(files)
    cmd.append(outfile)

    r = run_cmd(cmd)
    return make_ret(r)
