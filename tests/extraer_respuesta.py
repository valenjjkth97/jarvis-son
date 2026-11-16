#!/usr/bin/env python3
"""Extrae la respuesta limpia y la velocidad de generación de la salida de llama-cli (-st).
Uso: python3 tests/extraer_respuesta.py <archivo_salida>
Imprime dos líneas: la respuesta y los tokens/s de generación (o vacío)."""
import re, sys

texto = open(sys.argv[1], encoding="utf-8", errors="replace").read()
texto = re.sub(r"\x1b\[[0-9;?]*[A-Za-z]", "", texto)      # códigos de color
texto = re.sub(r"[^\n]*\r", "", texto)                     # líneas sobrescritas (spinner)
tps = re.search(r"Generation:\s*([0-9.]+)\s*t/s", texto)
corte = texto.find("[ Prompt:")
if corte != -1:
    texto = texto[:corte]
lineas = texto.split("\n")
inicio = 0
for i, l in enumerate(lineas):
    if l.startswith("> "):
        inicio = i + 1                                     # después del eco de la pregunta
resp = " ".join(l.strip() for l in lineas[inicio:] if l.strip() and l.strip() != "Exiting...")
print(resp)
print(tps.group(1) if tps else "")
