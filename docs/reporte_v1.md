# Jarvis-son V1 · Reporte de integración y evaluación (Etapa 7)

Fecha de la prueba: 2026-11-16, 00:20–00:34 · Equipo: Valentina Benítez (rol A) y Carlos Andrés Loaiza Escobar (rol B)
Asignatura: Inteligencia Artificial · COTECNOVA · Docente: Ing. Felipe Gutiérrez Isaza

## 1. Equipo de prueba

| Dato | Valor |
| --- | --- |
| Teléfono | Samsung Galaxy A36 5G (SM-A366E/DS) |
| SoC / CPU | Snapdragon 6 Gen 3 · 4 × Cortex-A78 + 4 × Cortex-A55 · ARM64 con NEON y producto punto |
| RAM | 8 GB (7,2 GiB visibles; 2,4 GiB disponibles antes de la prueba) · presupuesto para Jarvis: 3,0 GB |
| Android | 16 (One UI 8.0) |
| Entorno | Termux 0.118.3 + Termux:API 0.53.0 (F-Droid), clang 21.1.8, cmake 4.5.0 |

## 2. Configuración final

| Etapa | Herramienta | Modelo | Ajustes |
| --- | --- | --- | --- |
| Captura | Termux:API + FFmpeg 9.0.2 | — | 6 s, WAV PCM 16 bits mono 16 kHz |
| STT | whisper.cpp (`-DGGML_NATIVE=ON`) | Whisper base multilingüe | `-l es -t 4` |
| SLM | llama.cpp build 9fba51d (`-DGGML_NATIVE=ON`) | Qwen3.5-0.8B Q4_0 (GGUF oficial ggml-org) | contexto 1.024, salida 64, `-rea off`, `--repeat-penalty 1.2`, memoria de 1 turno |
| TTS | sherpa-onnx v1.13.8 + onnxruntime 1.31.1 | Piper `es_MX-ald-medium` | 4 hilos, 22.050 Hz |
| Orquestador | C++17 propio (`src/`) | — | fork + execvp, una etapa a la vez |

## 3. Prueba en modo avión

Todos los modelos y la voz se descargaron antes. Se activó el modo avión con Wi-Fi y Bluetooth apagados (ícono de avión visible en las capturas) y se ejecutó `./build/jarvis` directamente en Termux. Se hicieron **17 turnos seguidos** de voz a voz.

Datos crudos: `docs/resultados/turnos_modo_avion.csv` (solo métricas, sin texto de la conversación).

| Etapa | p50 | p95 | Máximo |
| --- | --- | --- | --- |
| STT (Whisper base) | 1.766 ms | 1.954 ms | 1.977 ms |
| SLM (Qwen3.5-0.8B) | 4.219 ms | 5.254 ms | 6.058 ms |
| TTS (Piper es_MX, síntesis) | 2.371 ms | 3.199 ms | 3.413 ms |
| **Respuesta (fin de la grabación → voz lista)** | **8.438 ms** | **10.153 ms** | 11.273 ms |
| Turno total (incluye 6 s de grabación y la reproducción) | 27.906 ms | 38.028 ms | 41.344 ms |

| Memoria pico por proceso | Valor |
| --- | --- |
| STT | 283 MB |
| SLM | 1.229 MB |
| TTS | 444 MB |
| Pico del sistema (etapas secuenciales) | **1.229 MB**, 41 % del presupuesto de 3 GB |

Velocidad media de generación del SLM: 19,1 tokens/s.

## 4. Criterios de aceptación

| Criterio de la guía | Resultado | Cumple |
| --- | --- | --- |
| 10 turnos consecutivos completos sin OOM ni pérdida del estado | 17/17 turnos completos, 0 errores, 0 cierres | Sí |
| ≥ 18/20 frases silenciosas transcritas de forma utilizable | 18/20 con Whisper base, WER medio 0,064 (`docs/resultados_stt.md`) | Sí |
| Cada respuesta se reproduce en voz; no se solapan TTS e inferencia | Las 17 respuestas sonaron; el orquestador lanza cada etapa solo cuando la anterior terminó | Sí |
| Funciona sin conexión, prueba en modo avión | 17 turnos en modo avión | Sí |
| El repositorio compila en CI y documenta instalación, licencias, checksums y límites | GitHub Actions compila y corre `test_parser`; README, `models/manifest.json` y este reporte | Sí |

## 5. Calidad observada (honestidad)

| Problema | Ejemplo de la prueba | Estado |
| --- | --- | --- |
| Errores de hecho del SLM | "Saturno es el planeta más grande"; año bisiesto confundido con "bicicleta" | Limitación de un modelo de 0,8B |
| Errores de STT con palabras poco comunes o dichas rápido | "hábito" → "álito"; "bisiesto" → "biciesto"; "un hábito" → "una evita" | Limitación de Whisper base; Whisper small queda como mejora |
| Silencio o ruido transcrito como "[Música]" y respondido como pregunta | Turnos 16 y 17 | **Corregido después de la prueba:** las transcripciones con solo marcas de ruido ahora cuentan como "no te escuché" |
| Respuesta cortada a mitad de palabra por el límite de 64 tokens | "…por eso, el uso exces" | **Corregido después de la prueba:** se recorta a la última frase completa |
| La memoria de un turno a veces mezcla el tema anterior | Tras "sistema experto", respondió "el término correcto es sistemas expertos" a otra pregunta | Se mantiene; se puede apagar con `memoria_turno_anterior = 0` |
| Copia parte de la instrucción del sistema | "sin usar emojis ni listas" dentro de una respuesta | Limitación del modelo pequeño |
| Espera fija de 6 s aunque se hable menos | Turno total p50 ≈ 28 s | Mejora futura: detección de voz (VAD) |

Comportamiento correcto ante un insulto ("¿por qué hablas tan mal?"): respondió sin agresividad.

## 6. Privacidad

- Los audios (`tmp/`) se borran al terminar o fallar cada turno.
- `logs/turnos.csv` guarda solo tiempos, memoria y estado, nunca texto.
- La memoria de un turno vive en RAM y desaparece al cerrar Jarvis.
- `.gitignore` bloquea pesos, audios y registros. No se suben IMEI ni datos del teléfono.

## 7. Cambios frente a la investigación previa

| Tema | Investigación | V1 final | Motivo |
| --- | --- | --- | --- |
| Orquestador | Python + asyncio con streaming | C++ secuencial | Lo exige la guía; el streaming queda para V2 |
| LLM | Qwen3.5-2B Q4_K_M | Qwen3.5-0.8B Q4_0 | Línea base de la guía; la comparación con 2B queda pendiente |
| STT | Whisper base | Whisper base | Confirmado con mediciones |
| TTS | Piper | Piper con sherpa-onnx v1.13.8 | sherpa-onnx 2.x ya no incluye el fonemizador |

## 8. Trabajo futuro

1. Etapa 8 "Hola Jarvis": detector de palabra de activación (openWakeWord) con el micrófono continuo de pulseaudio. Investigado en el plan; no implementado en V1.
2. Comparar Qwen3.5-2B Q4_K_M y Whisper small con el mismo corpus.
3. Detección de voz (VAD) para no esperar siempre 6 s.
4. Streaming LLM → TTS por frases (productor-consumidor), ya investigado en la fase 1.
5. Medir las 20 frases con ruido moderado, habla rápida y otro acento.
