#include <regex>
#include <sstream>

#include "jarvis.hpp"

namespace jarvis {

static std::string unir_lineas(const std::vector<std::string>& lineas) {
    std::string out;
    for (const auto& l : lineas) {
        const auto a = l.find_first_not_of(" \t");
        if (a == std::string::npos) continue;
        const std::string t = l.substr(a);
        if (t == "Exiting...") continue;
        if (!out.empty()) out += ' ';
        out += t;
    }
    while (!out.empty() && (out.back() == ' ' || out.back() == '\n')) out.pop_back();
    return out;
}

RespuestaSLM extraer_respuesta(const std::string& salida_llama) {
    RespuestaSLM r;
    // 1. quitar códigos de color ANSI y líneas sobrescritas con \r
    std::string t = std::regex_replace(salida_llama, std::regex("\x1b\\[[0-9;?]*[A-Za-z]"), "");
    t = std::regex_replace(t, std::regex("[^\n]*\r"), "");
    // 2. velocidad de generación
    std::smatch m;
    if (std::regex_search(t, m, std::regex("Generation:\\s*([0-9.]+)\\s*t/s"))) r.tokens_s = std::stod(m[1]);
    // 3. cortar en las estadísticas finales
    const auto corte = t.find("[ Prompt:");
    if (corte != std::string::npos) t = t.substr(0, corte);
    // 4. la respuesta empieza después del eco de la pregunta ("> ...")
    std::vector<std::string> lineas;
    std::istringstream in(t);
    std::string l;
    size_t inicio = 0;
    while (std::getline(in, l)) {
        lineas.push_back(l);
        if (l.rfind("> ", 0) == 0) inicio = lineas.size();
    }
    std::string resp = unir_lineas(std::vector<std::string>(lineas.begin() + static_cast<long>(inicio), lineas.end()));
    // 5. si se filtró razonamiento, quedarse con lo que viene después de </think>
    const auto fin_think = resp.rfind("</think>");
    if (fin_think != std::string::npos) {
        const std::string despues = resp.substr(fin_think + 8);
        if (despues.find_first_not_of(' ') != std::string::npos) resp = despues;
    }
    // 6. quitar etiquetas y marcas de markdown que el TTS leería en voz alta
    resp = std::regex_replace(resp, std::regex("</?think>|\\*\\*|__|#+ "), "");
    resp = std::regex_replace(resp, std::regex("\\s+"), " ");
    const auto a = resp.find_first_not_of(' ');
    const auto b = resp.find_last_not_of(' ');
    r.texto = a == std::string::npos ? "" : resp.substr(a, b - a + 1);
    return r;
}

std::string limpiar_transcripcion(const std::string& salida_whisper) {
    std::vector<std::string> lineas;
    std::istringstream in(salida_whisper);
    std::string l;
    while (std::getline(in, l)) lineas.push_back(l);
    return unir_lineas(lineas);
}

}  // namespace jarvis
