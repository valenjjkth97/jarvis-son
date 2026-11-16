# Jarvis-son V1

Asistente de voz local para Android que funciona por turnos: escucha una frase, la transcribe, genera una respuesta breve y la lee en voz alta. Todo corre en el teléfono con Termux, **sin internet** después de descargar los modelos.

Proyecto académico · Inteligencia Artificial · COTECNOVA (Cartago, Valle del Cauca), 2026 · Docente: Ing. Felipe Gutiérrez Isaza.

## Equipo

| Integrante | Rol | Responsabilidad |
| --- | --- | --- |
| Valentina Benítez | A | Termux, permisos, audio, FFmpeg, Whisper/STT y medición del teléfono |
| Carlos Andrés Loaiza Escobar | B | llama.cpp/Qwen, orquestador C++, TTS |
| Ambos | — | Contratos entre etapas, integración, revisión cruzada de PR, pruebas de aceptación |

## Cómo funciona

```
LISTO → GRABANDO → STT (whisper.cpp, Whisper base) → SLM (llama.cpp, Qwen3.5-0.8B Q4_0) → TTS (sherpa-onnx, Piper es_MX) → LISTO
                         cualquier fallo → ERROR → LISTO
```

Detalle de estados y contratos: [`docs/estados.md`](docs/estados.md).

## Resultados en el Galaxy A36 (modo avión)

| Métrica | Valor |
| --- | --- |
| Turnos seguidos sin errores | 17/17 |
| Respuesta (fin de la grabación → voz lista) | p50 8,4 s · p95 10,2 s |
| RAM pico (SLM) | 1.229 MB de un presupuesto de 3 GB |
| STT utilizable (20 frases en silencio) | 18/20, WER 0,064 |

Reporte completo: [`docs/reporte_v1.md`](docs/reporte_v1.md) · STT: [`docs/resultados_stt.md`](docs/resultados_stt.md) · SLM: [`docs/resultados_slm.md`](docs/resultados_slm.md) · TTS: [`docs/resultados_tts.md`](docs/resultados_tts.md).

## Instalación (Termux en Android 13+, ARM64)

Probado en Samsung Galaxy A36 5G, Android 16. Requiere unos 2 GB de almacenamiento libre.

**1. Apps:** instalar **Termux** y **Termux:API** desde F-Droid (las dos de la misma fuente). Dar permiso de micrófono a Termux:API.

**2. Paquetes:**
```
termux-wake-lock
pkg update -y && pkg upgrade -y
pkg install -y git cmake clang make ffmpeg termux-api procps time wget python onnxruntime bzip2
```

**3. whisper.cpp y modelo Whisper base:**
```
cd ~ && git clone --depth 1 https://github.com/ggml-org/whisper.cpp
cd ~/whisper.cpp && cmake -B build -DGGML_NATIVE=ON -DCMAKE_BUILD_TYPE=Release && cmake --build build -j 8
sh ./models/download-ggml-model.sh base
```

**4. llama.cpp y Qwen3.5-0.8B Q4_0:**
```
cd ~ && git clone --depth 1 https://github.com/ggml-org/llama.cpp
cd ~/llama.cpp && cmake -B build -DGGML_NATIVE=ON -DCMAKE_BUILD_TYPE=Release && cmake --build build -j 8 --target llama-cli llama-bench
mkdir -p ~/models && cd ~/models && wget -c https://huggingface.co/ggml-org/Qwen3.5-0.8B-GGUF/resolve/main/Qwen3.5-0.8B-Q4_0.gguf
```

**5. sherpa-onnx v1.13.8 y voz Piper es_MX** (la serie 2.x no sirve para voces Piper sin un fonemizador externo):
```
export SHERPA_ONNXRUNTIME_INCLUDE_DIR=$PREFIX/include/onnxruntime SHERPA_ONNXRUNTIME_LIB_DIR=$PREFIX/lib
cd ~ && git clone --depth 1 --branch v1.13.8 https://github.com/k2-fsa/sherpa-onnx sherpa-onnx-v1
cd ~/sherpa-onnx-v1 && cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSHERPA_ONNX_ENABLE_PORTAUDIO=OFF -DSHERPA_ONNX_ENABLE_WEBSOCKET=OFF -DSHERPA_ONNX_ENABLE_PYTHON=OFF -DSHERPA_ONNX_ENABLE_TESTS=OFF && cmake --build build -j 6 --target sherpa-onnx-offline-tts
cd ~/models && wget -c https://github.com/k2-fsa/sherpa-onnx/releases/download/tts-models/vits-piper-es_MX-ald-medium.tar.bz2 && tar xjf vits-piper-es_MX-ald-medium.tar.bz2
```

**6. Verificar descargas** contra los SHA-256 de [`models/manifest.json`](models/manifest.json):
```
sha256sum ~/whisper.cpp/models/ggml-base.bin ~/models/Qwen3.5-0.8B-Q4_0.gguf ~/models/vits-piper-es_MX-ald-medium/es_MX-ald-medium.onnx
```

**7. Compilar y ejecutar Jarvis:**
```
cd ~ && git clone https://github.com/valenjjkth97/jarvis-son && cd jarvis-son
cmake -B build && cmake --build build -j 4 && ./build/test_parser
./build/jarvis            # voz a voz: Enter para hablar, q para salir
./build/jarvis --texto    # escribir la pregunta (depuración de SLM + TTS)
python3 scripts/resumen_turnos.py   # p50/p95 por etapa desde logs/turnos.csv
```

Las rutas y ajustes están en [`config/jarvis.conf`](config/jarvis.conf). Para usar la voz de Android en lugar de Piper: `tts_modo = termux`.

## Estructura

| Ruta | Contenido |
| --- | --- |
| `src/` · `include/` | Orquestador C++ (estados, procesos, extracción de respuestas) |
| `config/jarvis.conf` | Rutas de modelos, contexto, tokens, instrucción del sistema, modo TTS |
| `scripts/` | Captura de audio y evaluación de STT, SLM y turnos |
| `models/manifest.json` | Origen, licencia, tamaño y SHA-256 de modelos y runtimes (no los pesos) |
| `docs/` | Inventario, estados, resultados por etapa y reporte final |
| `tests/` | Corpus de frases y preguntas, prueba unitaria del extractor |
| `.github/workflows/ci.yml` | CI: compila, corre pruebas y revisa los scripts |

## Limitaciones conocidas

- Qwen3.5-0.8B comete errores de hecho; Jarvis es una demostración académica, no una fuente confiable.
- Whisper base falla con palabras poco comunes o habla rápida.
- Cada turno graba 6 s fijos; la respuesta tarda unos 8–10 s después de hablar.
- La memoria de un turno a veces mezcla el tema anterior.
- No hay palabra de activación ("Hola Jarvis") ni interrupción de la voz en V1.
- Resultados medidos en un solo teléfono; no se extrapolan a otros equipos.

## Privacidad

Los audios temporales se borran en cada turno, el registro guarda solo métricas y no se suben pesos, grabaciones ni datos del teléfono.

## Licencias

Cada modelo, voz y runtime tiene su licencia (ver `models/manifest.json`): Whisper y whisper.cpp (MIT), llama.cpp (MIT), Qwen3.5-0.8B (Apache-2.0), sherpa-onnx (Apache-2.0), onnxruntime (MIT), voz Piper es_MX-ald (dataset Unlicense; revisar la voz base antes de redistribuir).
