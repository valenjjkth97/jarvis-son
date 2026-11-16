#!/usr/bin/env python3
"""Etapa 7 · Resumen de logs/turnos.csv (p50/p95 por etapa, RAM pico, errores).
Uso: python3 scripts/resumen_turnos.py [logs/turnos.csv]"""
import csv, sys

ruta = sys.argv[1] if len(sys.argv) > 1 else "logs/turnos.csv"
filas = list(csv.DictReader(open(ruta, encoding="utf-8")))
ok = [f for f in filas if f["estado"] == "ok"]

def pct(valores, q):
    v = sorted(valores)
    return v[min(len(v) - 1, int(round(q * (len(v) - 1))))] if v else 0

print(f"Turnos registrados: {len(filas)} | completos: {len(ok)} | "
      f"sin voz: {sum(f['estado'] == 'sin_voz' for f in filas)} | errores: {sum(f['estado'] == 'error' for f in filas)}")
print(f"{'Etapa':<26}{'p50 (ms)':>10}{'p95 (ms)':>10}{'máx (ms)':>10}")
for nombre, col in [("STT (whisper base)", "stt_ms"), ("SLM (Qwen3.5-0.8B)", "slm_ms"), ("TTS (Piper es_MX)", "tts_ms")]:
    v = [int(f[col]) for f in ok]
    print(f"{nombre:<26}{pct(v, .5):>10}{pct(v, .95):>10}{max(v, default=0):>10}")
resp = [int(f["stt_ms"]) + int(f["slm_ms"]) + int(f["tts_ms"]) for f in ok]
tot = [int(f["total_ms"]) for f in ok]
print(f"{'Respuesta (STT+SLM+TTS)':<26}{pct(resp, .5):>10}{pct(resp, .95):>10}{max(resp, default=0):>10}")
print(f"{'Turno total (con grabación)':<26}{pct(tot, .5):>10}{pct(tot, .95):>10}{max(tot, default=0):>10}")
for nombre, col in [("STT", "stt_rss_kb"), ("SLM", "slm_rss_kb"), ("TTS", "tts_rss_kb")]:
    print(f"RAM pico {nombre}: {max((int(f[col]) for f in ok), default=0) // 1024} MB")
tps = [float(f["gen_tps"]) for f in ok if float(f["gen_tps"]) > 0]
if tps:
    print(f"Generación SLM media: {sum(tps) / len(tps):.1f} t/s")
