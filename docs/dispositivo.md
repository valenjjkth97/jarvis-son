# Inventario del teléfono de prueba

Fecha del registro: 2026-11-15
Registrado por: Valentina Benítez (Termux vía SSH desde el PC)

| Dato | Valor | Cómo se obtuvo |
| --- | --- | --- |
| Modelo | Samsung Galaxy A36 5G (SM-A366E/DS) | Ajustes → Acerca del teléfono |
| Versión de Android | 16 (One UI 8.0) | Ajustes → Información de software |
| SoC | Qualcomm Snapdragon 6 Gen 3 (4 × Cortex-A78 + 4 × Cortex-A55) | Ficha técnica |
| Arquitectura | aarch64 (ARM64) | `uname -m` |
| Núcleos | 8 | `nproc` |
| Banderas de CPU relevantes | `asimd` (NEON), `asimddp` (producto punto), `asimdhp`, `asimdrdm` | `grep -m1 Features /proc/cpuinfo` |
| RAM total visible | 7,2 GiB | `free -h` |
| RAM disponible al medir | 2,4 GiB (con apps abiertas) | `free -h` |
| Swap (RAM Plus) | 8,0 GiB, 3,2 GiB en uso | `free -h` |
| Presupuesto para Jarvis | 3,0 GB (ver investigación previa) | — |
| Almacenamiento libre | 98 GB de 223 GB | `df -h ~` |
| Termux | 0.118.3 (F-Droid) | `termux-info` |
| Termux:API | 0.53.0 | `termux-info` |
| Compilador | clang 21.1.8 | `clang --version` |
| CMake | 4.5.0 | `cmake --version` |
| FFmpeg | 9.0.2 | `ffmpeg -version` |

## Observaciones

- La CPU tiene `asimddp` (producto punto), así que compilar con `-DGGML_NATIVE=ON` aprovecha esas instrucciones.
- Al medir, la RAM disponible era 2,4 GiB, menos que el presupuesto de 3,0 GB. Antes de cada prueba se cierran las apps en segundo plano y se vuelve a revisar con `free -h`.
- Prueba de micrófono con `termux-microphone-record` (5 s): grabó y se reprodujo correctamente.

No se registran IMEI, número de serie ni número de teléfono.
