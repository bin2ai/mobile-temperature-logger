# mobile-temperature-logger

## Software (SW)

setup python environment

```bash
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
```

build gui into exe

```bash
pyinstaller --onefile --windowed --icon=icon.png mtl_gui.py
```


run python gui from source

```bash
python mtl_gui.py
```
