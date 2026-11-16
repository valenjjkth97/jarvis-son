#!/data/data/com.termux/files/usr/bin/bash
# Etapa 4 · Evaluación del SLM con 10 preguntas cortas
# Uso: bash scripts/evaluar_slm.sh <modelo.gguf> [ctx] [max_tokens]
# Resultado: logs/slm_<modelo>.csv
set -euo pipefail
cd "$(dirname "$0")/.."

MODELO="${1:?Uso: evaluar_slm.sh <modelo.gguf> [ctx] [max_tokens]}"
CTX="${2:-1024}"
NMAX="${3:-96}"
LLAMA="${LLAMA_CLI:-$HOME/llama.cpp/build/bin/llama-cli}"
SYS="Eres Jarvis, un asistente de voz. Responde en español latino neutro, en máximo dos frases cortas. No hagas preguntas al final."
NOMBRE=$(basename "$MODELO" .gguf)
mkdir -p logs tmp
CSV="logs/slm_${NOMBRE}.csv"
echo "n,pregunta,respuesta,slm_ms,rss_max_kb,gen_tps,ctx,max_tokens" > "$CSV"

n=0
while IFS= read -r PREG <&3; do
  n=$((n + 1))
  echo; echo "[$n/10] $PREG"
  INI=$(date +%s%3N)
  /data/data/com.termux/files/usr/bin/time -f "%M" -o tmp/rss.txt \
    "$LLAMA" -m "$MODELO" -c "$CTX" -n "$NMAX" -t 4 -sys "$SYS" -p "$PREG" \
    -st --reasoning-budget 0 --simple-io > tmp/slm_out.txt 2>/dev/null < /dev/null || true
  FIN=$(date +%s%3N)
  RSS=$(tail -1 tmp/rss.txt)
  mapfile -t R < <(python3 tests/extraer_respuesta.py tmp/slm_out.txt)
  RESP="${R[0]:-}"; TPS="${R[1]:-}"
  echo "   -> $RESP"
  echo "   tiempo=$((FIN - INI)) ms  RAM pico=$((RSS / 1024)) MB  generación=${TPS} t/s"
  printf '%s,"%s","%s",%s,%s,%s,%s,%s\n' "$n" "$PREG" "${RESP//\"/}" "$((FIN - INI))" "$RSS" "$TPS" "$CTX" "$NMAX" >> "$CSV"
  rm -f tmp/slm_out.txt
done 3< tests/preguntas.txt

echo; echo "Listo: $CSV"
python3 - "$CSV" <<'PY'
import csv, sys
f = list(csv.DictReader(open(sys.argv[1], encoding="utf-8")))
t = sorted(int(x["slm_ms"]) for x in f)
p = lambda q: t[min(len(t) - 1, int(round(q * (len(t) - 1))))]
tps = [float(x["gen_tps"]) for x in f if x["gen_tps"]]
print(f"Preguntas: {len(f)} | Latencia SLM p50: {p(.5)} ms | p95: {p(.95)} ms | RAM pico máx: {max(int(x['rss_max_kb']) for x in f)//1024} MB")
if tps: print(f"Generación media: {sum(tps)/len(tps):.1f} t/s")
PY
