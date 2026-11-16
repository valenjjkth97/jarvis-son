# Etapa 5 · Resultados de la síntesis de voz (TTS)

Fecha: 2026-11-15 · Equipo: Samsung Galaxy A36 5G · Termux 0.118.3

## Prueba de viabilidad de sherpa-onnx en Termux

| Intento | Resultado |
| --- | --- |
| sherpa-onnx rama principal (v2.x) + onnxruntime 1.31.1 de Termux | Compila, pero no genera audio: desde v2.0.0 ignora `--vits-data-dir` y exige un fonemizador externo ("Failed to convert ... to token IDs") |
| **sherpa-onnx v1.13.8** (última 1.x) + onnxruntime 1.31.1 de Termux | **Compila y genera audio**. Trae espeak-ng incluido para convertir texto a fonemas |

Compilación: `cmake -DBUILD_SHARED_LIBS=ON -DSHERPA_ONNX_ENABLE_PORTAUDIO=OFF -DSHERPA_ONNX_ENABLE_WEBSOCKET=OFF -DSHERPA_ONNX_ENABLE_PYTHON=OFF -DSHERPA_ONNX_ENABLE_TESTS=OFF`, objetivo `sherpa-onnx-offline-tts`, con `SHERPA_ONNXRUNTIME_INCLUDE_DIR` y `SHERPA_ONNXRUNTIME_LIB_DIR` apuntando al paquete `onnxruntime` de Termux.

## Voz elegida

| Dato | Valor |
| --- | --- |
| Voz | `vits-piper-es_MX-ald-medium` (Piper, español de México, 1 hablante) |
| Frecuencia | 22.050 Hz |
| Dataset | Ald_Mexican_Spanish_speech_dataset (Unlicense, según el MODEL_CARD) |
| Origen | Afinada desde la voz española davefx de Piper |
| SHA-256 del .onnx | `b8198f4e0495029a686db2f7a7cb70bd4b3d6fe4300e3ee743d8a82602194389` |

## Medición (frase: "Hola, soy Jarvis. La capital de Colombia es Bogotá.")

| Métrica | sherpa-onnx + Piper es_MX | Respaldo `termux-tts-speak` |
| --- | --- | --- |
| Tiempo de síntesis (sin contar la carga) | 0,468 s | — (sintetiza y reproduce a la vez) |
| Tiempo total del proceso (carga + síntesis) | 2,35 s | 9,7–21,9 s por turno (incluye la reproducción) |
| Duración del audio | 4,44 s | — |
| RTF | **0,105** (10 veces más rápido que el audio) | — |
| RAM pico | 178 MB | — (lo maneja el motor de Android) |
| Funciona sin la voz de Google | Sí, todo es local | Depende del motor instalado en Android |

## Evaluación subjetiva

- Se entiende bien y suena más rápido que el respaldo de Android.
- Acento mexicano neutro, cercano al español latino que pide la guía.
- La prueba en modo avión del turno completo se hace en la Etapa 7.

## Decisión

Jarvis-son V1 usa **sherpa-onnx v1.13.8 + Piper es_MX-ald-medium** (`tts_modo = sherpa` en `config/jarvis.conf`). `termux-tts-speak` queda como respaldo de demostración (`tts_modo = termux`), sin garantía de funcionar sin red en todos los teléfonos.

## Limitaciones

- La voz base de Piper (davefx) tiene su propia licencia; hay que revisarla antes de redistribuir el modelo. El repositorio no incluye los pesos, solo el enlace y el SHA-256.
- sherpa-onnx 2.x requiere otro fonemizador; actualizar exigirá adaptar el comando.
