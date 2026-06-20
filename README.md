
## How to use

1. Clone repository
```
git clone git@github.com:embedakk/AERO_MAIN_SYS.git
```

2. Go inside of project folder
```
cd AERO_MAIN_SYS
```

3. Start VSC
```
code .
```

## Config, Build and Flash

```
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p (PORT) flash
```
