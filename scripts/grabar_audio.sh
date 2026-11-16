#!/data/data/com.termux/files/usr/bin/bash
# Etapa 2 · Captura y normalización de audio
# Uso: scripts/grabar_audio.sh <salida.wav> [segundos]
# Graba con Termux:API, convierte a WAV PCM 16 bits, mono, 16 kHz y borra el temporal.
set -euo pipefail

SALIDA="${1:?Uso: grabar_audio.sh <salida.wav> [segundos]}"
SEG="${2:-6}"
if ! [[ "$SEG" =~ ^[0-9]+$ ]] || (( SEG < 1 || SEG > 15 )); then
  echo "ERROR: la duración debe estar entre 1 y 15 segundos" >&2; exit 2
fi

TMP_DIR="${TMPDIR:-$PREFIX/tmp}/jarvis"
mkdir -p "$TMP_DIR" "$(dirname "$SALIDA")"
CRUDO="$TMP_DIR/crudo_$$.m4a"
trap 'rm -f "$CRUDO"' EXIT

termux-microphone-record -q >/dev/null 2>&1 || true   # por si quedó una grabación abierta
echo ">> Habla ahora ($SEG s)..." >&2
termux-microphone-record -f "$CRUDO" -l "$SEG" >/dev/null
sleep $((SEG + 1))
termux-microphone-record -q >/dev/null 2>&1 || true

if [[ ! -s "$CRUDO" ]]; then
  echo "ERROR: no se creó la grabación. ¿Permiso de micrófono en Termux:API?" >&2; exit 3
fi

ffmpeg -nostdin -loglevel error -y -i "$CRUDO" -ac 1 -ar 16000 -c:a pcm_s16le "$SALIDA"

# Validación del formato
INFO=$(ffprobe -v error -select_streams a:0 \
  -show_entries stream=codec_name,sample_rate,channels -show_entries format=duration \
  -of default=noprint_wrappers=1 "$SALIDA")
echo "$INFO" | grep -q "codec_name=pcm_s16le" || { echo "ERROR: códec incorrecto" >&2; exit 4; }
echo "$INFO" | grep -q "sample_rate=16000"    || { echo "ERROR: frecuencia incorrecta" >&2; exit 4; }
echo "$INFO" | grep -q "channels=1"           || { echo "ERROR: no es mono" >&2; exit 4; }
echo "$INFO" >&2
echo "$SALIDA"
