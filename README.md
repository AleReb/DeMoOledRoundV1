# Demo OLED/TFT redonda GC9A01 con ESP32

Este proyecto contiene demos para una pantalla redonda GC9A01 de 1.28" y 240x240 pixeles usando Arduino + ESP32.

Autor:

- Alejandro Rebolledo
- arebolledo@udd.cl

Hay dos sketches:

- `Demo_GC9A01/Demo_GC9A01.ino`: demo simple de la pantalla por SPI, sin archivos externos.
- `DeMO_GIF_GC9A01/DeMO_GIF_GC9A01.ino`: demo con animacion tipo sprite. Carga frames `.raw` desde LittleFS. Esta animacion es una prueba de concepto.

## Video demo

Puedes ver el video de demostracion aqui:

![Ver demo](docs/demo.mp4)

Tambien hay un conversor:

- `python_Converter/Converter_raw.py`: convierte imagenes `png/cat*.png` a archivos `.raw` RGB565 little-endian.

## Hardware

Pantalla usada por el codigo:

- Controlador: `GC9A01`
- Resolucion: `240x240`
- Interfaz: SPI de 4 hilos
- Backlight controlado por GPIO

## Cableado usado en los sketches

Los GPIO estan definidos al comienzo de ambos archivos `.ino`:

| Senal pantalla | GPIO ESP32 |
| --- | ---: |
| `SCLK` / `SCK` | `12` |
| `MOSI` / `SDA` / `DIN` | `11` |
| `CS` | `8` |
| `DC` | `10` |
| `RST` / `RES` | `9` |
| `BL` / `LED` | `13` |
| `GND` | `GND` |
| `VCC` | `3V3` |

Salvedades importantes:

- Estos GPIO corresponden al montaje usado en este proyecto. Si tu placa ESP32 usa otros pines disponibles, cambia los `#define TFT_*` en el sketch.
- No todos los ESP32 permiten usar los mismos GPIO. En muchos modulos ESP32 clasicos tipo WROOM, los GPIO `6` a `11` estan reservados para la memoria flash y no deben usarse para la pantalla. En ese caso cambia el cableado y los `#define` por pines libres de tu placa.
- Revisa tambien si tu placa expone los GPIO `8`, `9`, `10`, `11`, `12` y `13`. En placas ESP32-C3, ESP32-S2 o ESP32-S3 esto depende del modelo exacto de placa.
- El codigo usa SPI por hardware con pines personalizados:

```cpp
SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
```

- No se usa `MISO`, por eso se pasa `-1`.
- El backlight se activa con `HIGH`:

```cpp
digitalWrite(TFT_BL, HIGH);
```

Si tu modulo de pantalla tiene el backlight siempre conectado a VCC, el pin `TFT_BL` puede no ser necesario. Si el backlight de tu modulo es activo en bajo, cambia esa linea a `LOW`.

## Librerias necesarias

Instala desde el Library Manager de Arduino IDE:

- `Adafruit GFX Library`
- `Adafruit GC9A01A`
- Dependencias que Arduino IDE pida instalar junto con esas librerias.

Tambien necesitas tener instalado el core de ESP32 para Arduino, porque el demo de animacion usa:

```cpp
#include <LittleFS.h>
```

Para el demo con animacion, selecciona una configuracion de particiones que deje espacio para filesystem. Si el IDE o el plugin no pueden subir `data`, revisa el menu de particiones de la placa y elige una opcion que incluya LittleFS/SPIFFS.

## Carga del demo simple

1. Abre `Demo_GC9A01/Demo_GC9A01.ino` en Arduino IDE.
2. Selecciona tu placa ESP32 y el puerto correcto.
3. Verifica que los GPIO del cableado coincidan con los `#define`.
4. Sube el sketch.
5. Abre el monitor serial a `115200`.

Este demo no requiere subir archivos LittleFS.

## Carga del demo con animacion desde LittleFS

El sketch `DeMO_GIF_GC9A01/DeMO_GIF_GC9A01.ino` espera encontrar estos archivos dentro de LittleFS:

```text
/cat/cat00.raw
/cat/cat01.raw
/cat/cat02.raw
/cat/cat03.raw
```

En el proyecto ya estan en:

```text
DeMO_GIF_GC9A01/data/cat/
```

Cada frame mide `120 x 130` pixeles en RGB565, por lo que cada archivo `.raw` debe pesar `31200` bytes.

El sketch carga los 4 frames a RAM al iniciar. Eso usa aproximadamente `124800` bytes solo para los sprites. Si aparece `RAM allocation failed`, reduce la cantidad de frames, el tamano del sprite o usa una placa con mas RAM disponible.

Pasos:

1. Abre `DeMO_GIF_GC9A01/DeMO_GIF_GC9A01.ino`.
2. Selecciona tu placa ESP32 y el puerto correcto.
3. Sube el sketch.
4. Sube la carpeta `data` a LittleFS usando el plugin correspondiente para tu version de Arduino IDE.
5. Reinicia la placa.
6. Abre el monitor serial a `115200` para revisar el estado de la pantalla, LittleFS y los sprites.

Si LittleFS no monta o faltan los archivos `.raw`, el sketch muestra un dibujo de respaldo y escribe el error por serial.

## Subir archivos a LittleFS desde Arduino IDE

La carpeta `data` que se sube debe estar al lado del `.ino`, por ejemplo:

```text
DeMO_GIF_GC9A01/
  DeMO_GIF_GC9A01.ino
  data/
    cat/
      cat00.raw
      cat01.raw
      cat02.raw
      cat03.raw
```

Para Arduino IDE 2.x, el plugin que se ha usado correctamente es:

- [earlephilhower/arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)

Salvedad: en este proyecto funciona bien hasta Arduino IDE `2.3.4`.

Para Arduino IDE clasico `1.8.x`, puedes usar:

- [lorol/arduino-esp32fs-plugin](https://github.com/lorol/arduino-esp32fs-plugin)

Despues de instalar el plugin, usa la opcion de subida LittleFS/ESP32 Sketch Data Upload que aparezca en el IDE. Primero compila/sube el sketch y luego sube `data`.

## Velocidad SPI

El demo con animacion configura la pantalla a `40 MHz`:

```cpp
tft.setSPISpeed(40000000);
```

Si ves ruido, colores incorrectos, pantallazos o artefactos, baja la velocidad, por ejemplo:

```cpp
tft.setSPISpeed(27000000);
```

## Regenerar los frames RAW

El conversor requiere Python y Pillow:

```powershell
pip install pillow
```

Desde la carpeta `python_Converter`, ejecuta:

```powershell
python Converter_raw.py
```

El script toma los PNG desde:

```text
python_Converter/png/
```

y genera los `.raw` en:

```text
python_Converter/data/cat/
```

Para usar los nuevos frames en el sketch de animacion, copia los `.raw` generados a:

```text
DeMO_GIF_GC9A01/data/cat/
```

Luego vuelve a subir la carpeta `data` a LittleFS.

Detalles del formato:

- Tamano de sprite: `120 x 130`
- Formato de color: RGB565
- Orden de bytes: little-endian
- Color transparente: magenta `RGB(255, 0, 255)`, equivalente a `0xF81F`

El sketch reemplaza ese magenta por el color de fondo local para dibujar el sprite rapido con `drawRGBBitmap`.

## Diagnostico por monitor serial

Configura el monitor serial a `115200`.

El demo con animacion reporta:

- Estado de la pantalla.
- Estado del backlight.
- Estado de LittleFS.
- Si los sprites cargaron correctamente.
- Frame actual.

Mensajes comunes:

- `LittleFS mounted.`: LittleFS monto correctamente.
- `Cannot open frame`: falta un archivo `.raw` o no se subio `data`.
- `Invalid frame size`: el `.raw` no mide `31200` bytes.
- `Some frames failed to load. Fallback will be used.`: el sketch usara el dibujo de respaldo.

## Archivos principales

```text
Demo_GC9A01/
  Demo_GC9A01.ino

DeMO_GIF_GC9A01/
  DeMO_GIF_GC9A01.ino
  data/cat/*.raw

python_Converter/
  Converter_raw.py
  png/*.png
  data/cat/*.raw
```
