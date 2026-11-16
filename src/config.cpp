#include <cstdlib>
#include <fstream>

#include "jarvis.hpp"

namespace jarvis {

std::string expandir_home(const std::string& ruta) {
    if (ruta.rfind("~/", 0) == 0) {
        const char* home = std::getenv("HOME");
        return std::string(home ? home : "") + ruta.substr(1);
    }
    return ruta;
}

static std::string recortar(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    const auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

Config cargar_config(const std::string& ruta) {
    Config c;
    std::ifstream f(ruta);
    std::string linea;
    while (std::getline(f, linea)) {
        linea = recortar(linea);
        if (linea.empty() || linea[0] == '#') continue;
        const auto eq = linea.find('=');
        if (eq == std::string::npos) continue;
        c.v[recortar(linea.substr(0, eq))] = recortar(linea.substr(eq + 1));
    }
    return c;
}

std::string Config::get(const std::string& clave, const std::string& defecto) const {
    const auto it = v.find(clave);
    return it == v.end() ? defecto : expandir_home(it->second);
}

int Config::get_int(const std::string& clave, int defecto) const {
    const auto it = v.find(clave);
    if (it == v.end()) return defecto;
    try {
        return std::stoi(it->second);
    } catch (...) {
        return defecto;
    }
}

const char* nombre_estado(Estado e) {
    switch (e) {
        case Estado::Listo: return "LISTO";
        case Estado::Grabando: return "GRABANDO";
        case Estado::Transcribiendo: return "STT";
        case Estado::Generando: return "SLM";
        case Estado::Hablando: return "TTS";
        case Estado::Error: return "ERROR";
    }
    return "?";
}

}  // namespace jarvis
