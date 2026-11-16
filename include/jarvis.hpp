// Jarvis-son V1 · interfaces del orquestador
#pragma once
#include <map>
#include <string>
#include <vector>

namespace jarvis {

// Estados de la máquina de estados (Etapa 6)
enum class Estado { Listo, Grabando, Transcribiendo, Generando, Hablando, Error };
const char* nombre_estado(Estado e);

// Configuración leída de config/jarvis.conf (clave=valor)
struct Config {
    std::map<std::string, std::string> v;
    std::string get(const std::string& clave, const std::string& defecto = "") const;
    int get_int(const std::string& clave, int defecto) const;
};
Config cargar_config(const std::string& ruta);
std::string expandir_home(const std::string& ruta);

// Resultado de ejecutar un proceso local
struct ResultadoProceso {
    int codigo = -1;          // código de salida (o -1 si no se pudo lanzar)
    std::string salida;       // stdout capturado
    long rss_max_kb = 0;      // memoria pico del proceso hijo (wait4 / ru_maxrss)
    long ms = 0;              // duración en milisegundos
};

// Lanza un programa con argumentos fijos (fork + execvp). Nunca usa la shell:
// el texto transcrito viaja como un argumento más, así se evita shell injection.
ResultadoProceso ejecutar(const std::vector<std::string>& argv, bool capturar_stdout = true);

// Extrae la respuesta limpia de la salida de llama-cli (-st) y los tokens/s de generación
struct RespuestaSLM {
    std::string texto;
    double tokens_s = 0.0;
};
RespuestaSLM extraer_respuesta(const std::string& salida_llama);

// Quita emojis y símbolos gráficos para que el TTS no los lea en voz alta
std::string quitar_emojis(const std::string& s);

// Limpia la transcripción de whisper-cli (espacios y saltos de línea)
std::string limpiar_transcripcion(const std::string& salida_whisper);

}  // namespace jarvis
