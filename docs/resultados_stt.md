# Etapa 3 · Resultados del reconocimiento de voz (STT)

Fecha: 2026-11-15 · Equipo: Samsung Galaxy A36 5G (Snapdragon 6 Gen 3, 8 GB) · Termux 0.118.3
Motor: whisper.cpp compilado en el teléfono con `-DGGML_NATIVE=ON`, 4 hilos (`-t 4`), idioma `-l es`.
Corpus: 20 frases de `tests/frases.txt`, grabadas en silencio, clips de 6 s, WAV mono 16 kHz.
Criterio de "utilizable": WER ≤ 0,25 frente al texto de referencia (sin tildes ni signos).

## Comparación

| Métrica | Whisper tiny (multilingüe) | Whisper base (multilingüe) |
| --- | --- | --- |
| Frases utilizables | 15/20 | **18/20** (cumple la meta ≥ 18) |
| WER medio | 0,163 | **0,064** |
| Latencia STT p50 | 1.126 ms | 1.831 ms |
| Latencia STT p95 | 1.312 ms | 3.334 ms |
| RTF aproximado (p50 / 6,14 s de audio) | 0,18 | 0,30 |
| RAM pico observada | 175 MB | 283 MB |

Datos crudos: `docs/resultados/stt_ggml-tiny_silencio.csv` y `docs/resultados/stt_ggml-base_silencio.csv`.

## Decisión

Jarvis-son V1 usa **Whisper base**. Tiny no alcanzó la meta de 18/20, tal como prevé la guía ("comparar base solo si tiny queda corto"). Base sí la cumple, reduce el WER a menos de la mitad y sigue siendo más rápido que el audio (RTF < 1). Sus 283 MB de RAM caben con holgura en el presupuesto de 3 GB.

## Errores observados

- Nombres propios y títulos: "Cien años de soledad" → "el final de solidar" (tiny) y "100 años de solidar" (base).
- La palabra de activación: "Jarvis" → "Jordi" con tiny; base la reconoce bien.
- Números escritos con cifras ("100" por "cien") cuentan como error en el WER aunque el sentido se conserva.

## Limitaciones

- Un solo hablante, a 20–30 cm, en una habitación silenciosa.
- Las condiciones de ruido moderado, habla rápida y otros acentos quedan pendientes de medir con el mismo script (`bash scripts/evaluar_stt.sh <modelo> ruido 6`).
- Los audios no se guardan: se borran después de cada transcripción.
