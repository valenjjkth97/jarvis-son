# Jarvis-son V1

Asistente de voz local para Android que funciona por turnos: escucha una frase, la transcribe, genera una respuesta breve y la lee en voz alta. Todo se ejecuta en el teléfono con Termux, sin conexión después de descargar los modelos.

Proyecto académico de la asignatura Inteligencia Artificial, COTECNOVA (Cartago, Valle del Cauca), 2026.
Docente: Ing. Felipe Gutiérrez Isaza.

## Equipo

| Integrante | Rol | Responsabilidad |
| --- | --- | --- |
| Valentina Benítez | Por definir (A o B) | |
| Carlos Andrés Loaiza Escobar | Por definir (A o B) | |

- **Rol A:** Termux, permisos, audio, FFmpeg, Whisper/STT y medición del teléfono.
- **Rol B:** llama.cpp/Qwen, orquestador C++, TTS, documentación y release.

## Flujo

```
LISTO → GRABANDO → STT (whisper.cpp) → SLM (llama.cpp + Qwen) → TTS (Piper/sherpa-onnx) → LISTO
                        cualquier fallo → ERROR → LISTO
```

## Estado del proyecto

| Etapa | Estado |
| --- | --- |
| 0 · Organizar el proyecto | En curso |
| 1 · Preparar Termux | Pendiente |
| 2 · Capturar y normalizar audio | Pendiente |
| 3 · STT | Pendiente |
| 4 · SLM | Pendiente |
| 5 · TTS | Pendiente |
| 6 · Orquestador C++ | Pendiente |
| 7 · Integración y evaluación | Pendiente |
| 8 · "Hola Jarvis" (opcional) | Pendiente |

## Estructura

| Ruta | Contenido |
| --- | --- |
| `src/` · `include/` | Orquestador, interfaces y configuración |
| `scripts/` | Instalación, descarga verificada y utilidades |
| `models/manifest.json` | Nombre, origen, licencia y checksum de cada modelo (no los pesos) |
| `docs/` | Inventario del teléfono, diagrama, decisiones y métricas |
| `tests/` | Frases de referencia para las pruebas |

## Reglas del repositorio

- No se suben pesos de modelos (`.gguf`, `.bin`, `.onnx`) ni grabaciones de voz.
- No se suben tokens, contraseñas ni identificadores del teléfono (IMEI, número de serie).
- Todo cambio entra por pull request y lo revisa la persona que no lo hizo.

## Instalación

Se completa en la Etapa 1.

## Licencias

Ver `models/manifest.json`. Cada modelo, voz y herramienta tiene su propia licencia.
