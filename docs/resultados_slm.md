# Etapa 4 · Resultados del modelo de lenguaje (SLM)

Fecha: 2026-11-15 · Equipo: Samsung Galaxy A36 5G (Snapdragon 6 Gen 3, 8 GB) · Termux 0.118.3
Motor: llama.cpp (build 9fba51d) compilado en el teléfono con `-DGGML_NATIVE=ON`, 4 hilos.
Modelo: Qwen3.5-0.8B Q4_0 (GGUF oficial de ggml-org, 563.036.064 bytes, SHA-256 en `models/manifest.json`).
Configuración: contexto 1.024 tokens, salida máxima 96 tokens, una pregunta por proceso (`-st`).
Corpus: 10 preguntas cortas de `tests/preguntas.txt`.

## Velocidad base (llama-bench, 4 hilos)

| Prueba | Tokens/s |
| --- | --- |
| Lectura del prompt (pp64) | 199,4 ± 1,7 |
| Generación (tg64) | 19,9 ± 0,2 |

## Comparación de dos configuraciones de decodificación

| Métrica | Prueba 1 · `--reasoning-budget 0` | Prueba 2 · ajustada (`-rea off --repeat-penalty 1.2`, instrucción más corta) |
| --- | --- | --- |
| Latencia por pregunta p50 | 5.301 ms | 5.424 ms |
| Latencia por pregunta p95 | 8.609 ms | 14.288 ms (*) |
| Generación media | 17,6 t/s | 17,2 t/s |
| RAM pico observada | 1.222 MB | 1.219 MB |
| Respuestas con bucles de repetición | 3/10 | 0/10 |
| Respuestas con etiqueta `</think>` filtrada | 2/10 | 0/10 |
| Respuestas que copian la instrucción del sistema | 3/10 | 0/10 |
| Respuestas con error de contenido evidente | 3/10 (planeta, año bisiesto, sistema experto) | 3/10 (planeta "Marte", año bisiesto "365 días", sistema experto) |

(*) La pregunta 8 tardó 14,3 s y generó a 6,8 t/s, muy por debajo del resto (15–19,6 t/s). Coincidió con la instalación de paquetes en otra sesión de Termux, así que se trata como interferencia y no como comportamiento normal del modelo. Sin esa pregunta, el máximo fue 7,4 s.

Datos crudos: `docs/resultados/slm_Qwen3.5-0.8B-Q4_0_base.csv` (prueba 1) y `docs/resultados/slm_Qwen3.5-0.8B-Q4_0_ajustado.csv` (prueba 2).

## Decisión

Jarvis-son V1 usa la **configuración ajustada** (`razonamiento = off`, `penalizacion_repeticion = 1.2` en `config/jarvis.conf`). Elimina los bucles, la filtración de `</think>` y la copia de la instrucción, con la misma velocidad y memoria. El extractor de respuestas también quita `</think>` y marcas de markdown para que el TTS no las lea.

## Presupuesto de memoria medido

| Componente | RAM pico medida |
| --- | --- |
| Whisper base (STT) | 283 MB |
| Qwen3.5-0.8B Q4_0, contexto 1.024 (SLM) | 1.219 MB |
| Máximo simultáneo (las etapas son secuenciales) | 1.219 MB |
| Presupuesto estricto | 3.000 MB |

Como cada etapa termina antes de que empiece la siguiente, el pico del sistema es el del SLM, no la suma.

## Limitaciones

- Un modelo de 0,8B comete errores de hecho (por ejemplo, "el planeta más grande es Marte"). Jarvis-son V1 es una demostración académica y no debe usarse como fuente confiable.
- Algunas respuestas superan las dos frases pedidas y se cortan en 96 tokens.
- La comparación con Qwen3.5-2B Q4_K_M (propuesta de la investigación previa) queda como trabajo futuro.
