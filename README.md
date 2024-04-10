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
