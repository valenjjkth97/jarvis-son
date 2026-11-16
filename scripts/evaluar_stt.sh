#!/data/data/com.termux/files/usr/bin/bash
# Etapa 3 · Evaluación de STT con las 20 frases
# Uso: scripts/evaluar_stt.sh <modelo.bin> [condicion] [segundos]
# Ej.:  scripts/evaluar_stt.sh ~/whisper.cpp/models/ggml-tiny.bin silencio 6
# Resultado: logs/stt_<modelo>_<condicion>.csv  (no guarda audios)
set -euo pipefail
cd "$(dirname "$0")/.."

MODELO="${1:?Uso: evaluar_stt.sh <modelo.bin> [condicion] [segundos]}"
COND="${2:-silencio}"
SEG="${3:-6}"
WHISPER="${WHISPER_CLI:-$HOME/whisper.cpp/build/bin/whisper-cli}"
FRASES="tests/frases.txt"
NOMBRE=$(basename "$MODELO" .bin)
mkdir -p logs tmp
CSV="logs/stt_${NOMBRE}_${COND}.csv"
echo "n,condicion,referencia,transcripcion,wer,stt_ms,rss_max_kb,audio_s" > "$CSV"

n=0
while IFS= read -r REF <&3; do
  n=$((n + 1))
  echo; echo "[$n/20] Di: \"$REF\"  (Enter para grabar)"; read -r _
  WAV="tmp/stt_$n.wav"
  bash scripts/grabar_audio.sh "$WAV" "$SEG" >/dev/null
  AUD=$(ffprobe -v error -show_entries format=duration -of csv=p=0 "$WAV")
  INI=$(date +%s%3N)
  HYP=$(/data/data/com.termux/files/usr/bin/time -f "%M" -o tmp/rss.txt \
        "$WHISPER" -m "$MODELO" -f "$WAV" -l es -t 4 -np -nt 2>/dev/null | tr '\n' ' ' | sed 's/^ *//;s/ *$//')
  FIN=$(date +%s%3N)
  RSS=$(tail -1 tmp/rss.txt)
  W=$(python3 tests/wer.py "$REF" "$HYP")
  rm -f "$WAV"
  echo "   Transcripción: $HYP"
  echo "   WER=$W  tiempo=$((FIN - INI)) ms  RAM pico=$((RSS / 1024)) MB"
  printf '%s,%s,"%s","%s",%s,%s,%s,%s\n' "$n" "$COND" "$REF" "${HYP//\"/}" "$W" "$((FIN - INI))" "$RSS" "$AUD" >> "$CSV"
done 3< "$FRASES"

echo; echo "Listo: $CSV"
python3 - "$CSV" <<'PY'
import csv, sys, statistics as st
filas = list(csv.DictReader(open(sys.argv[1], encoding="utf-8")))
w = [float(f["wer"]) for f in filas]; t = sorted(int(f["stt_ms"]) for f in filas)
usables = sum(1 for x in w if x <= 0.25)
p = lambda q: t[min(len(t) - 1, int(round(q * (len(t) - 1))))]
print(f"Frases: {len(filas)} | Utilizables (WER <= 0,25): {usables}/{len(filas)} | WER medio: {st.mean(w):.3f}")
print(f"Latencia STT p50: {p(.5)} ms | p95: {p(.95)} ms | RAM pico máx: {max(int(f['rss_max_kb']) for f in filas)//1024} MB")
PY
