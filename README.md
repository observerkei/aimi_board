### Features

1. **Speech Content Understanding:** Parse the user's input speech content.
2. **Speech Playback:** Play synthesized speech output.
3. **Semantic Recognition:** Understand the semantics within the speech content.
4. **Speech Dialogue API:** Engage in intelligent dialogue through speech.
5. **Button Control:** Support interaction via button controls.
6. **Screen Display:** Display relevant information on the screen.

### How to Use

1. **Register and Obtain API Keys**

```bash
export OPENAI_API_KEY=your-openai-key
export SPEECH_KEY=your-azure-key
export SPEECH_REGION=your-azure-key-region
```

2. **Configuring the Runtime Environment**

Prepare a device, such as `MaixII-Sense`, or any device containing a microphone, speakers, display (with framebuffer driver support), and buttons (partial inclusion is also acceptable).

```bash
sudo bash -x build_environment.sh
```

3. **Running the Program**

use

```bash
python3 voice_assistant.py 
```

or

```bash
sudo python3 voice_assistant.py
```


### Doc

[Deploy and build documentation](https://blog.observerkei.top/WB/Develop/Embedded/Voice-Assistant/%E9%83%A8%E7%BD%B2%E5%92%8C%E6%9E%84%E5%BB%BA)


