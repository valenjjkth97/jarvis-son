#!/usr/bin/env python3
"""Calcula WER (Word Error Rate) entre una referencia y una transcripción.
WER = (sustituciones + borrados + inserciones) / palabras de la referencia.
Uso: python3 tests/wer.py "texto referencia" "texto transcrito"
"""
import re, sys, unicodedata

def normalizar(t):
    t = unicodedata.normalize("NFD", t.lower())
    t = "".join(c for c in t if unicodedata.category(c) != "Mn")  # quita tildes
    t = re.sub(r"[^a-zñ0-9 ]", " ", t)
    return t.split()

def wer(ref, hyp):
    r, h = normalizar(ref), normalizar(hyp)
    d = [[0] * (len(h) + 1) for _ in range(len(r) + 1)]
    for i in range(len(r) + 1): d[i][0] = i
    for j in range(len(h) + 1): d[0][j] = j
    for i in range(1, len(r) + 1):
        for j in range(1, len(h) + 1):
            costo = 0 if r[i-1] == h[j-1] else 1
            d[i][j] = min(d[i-1][j] + 1, d[i][j-1] + 1, d[i-1][j-1] + costo)
    return d[len(r)][len(h)] / max(len(r), 1)

if __name__ == "__main__":
    print(f"{wer(sys.argv[1], sys.argv[2]):.3f}")
