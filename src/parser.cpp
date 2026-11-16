#include <regex>
#include <sstream>

#include "jarvis.hpp"

namespace jarvis {

// Quita emojis y símbolos que el TTS leería por su nombre ("cara sonriente", etc.).
// Se eliminan: secuencias UTF-8 de 4 bytes (U+10000 en adelante, donde están casi todos los emojis),
// símbolos varios y dingbats (U+2600–U+27BF), el selector de variación U+FE0F y el unidor U+200D.
std::string quitar_emojis(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size();) {
        const auto c = static_cast<unsigned char>(s[i]);
        size_t len = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
        if (i + len > s.size()) len = 1;
        bool quitar = len == 4;
        if (len == 3) {
            const auto c1 = static_cast<unsigned char>(s[i + 1]), c2 = static_cast<unsigned char>(s[i + 2]);
            quitar = (c == 0xE2 && c1 >= 0x98 && c1 <= 0x9E) ||                 // U+2600–U+27BF
                     (c == 0xEF && c1 == 0xB8 && c2 == 0x8F) ||                 // U+FE0F
                     (c == 0xE2 && c1 == 0x80 && c2 == 0x8D);                   // U+200D
        }
        if (!quitar) out.append(s, i, len);
        i += len;
    }
    return out;
}

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
    resp = quitar_emojis(resp);
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
    std::string t = unir_lineas(lineas);
    // Whisper marca el ruido o el silencio como [Música], [MÚSICA], (aplausos), [BLANK_AUDIO]...
    // Si la transcripción es solo eso, no hubo voz.
    const std::string sin_marcas = std::regex_replace(t, std::regex("\\[[^\\]]*\\]|\\([^)]*\\)|[ .,]"), "");
    return sin_marcas.empty() ? "" : t;
}

std::string recortar_frase_incompleta(const std::string& texto) {
    if (texto.empty()) return texto;
    const std::string fin = ".?!\"";
    if (fin.find(texto.back()) != std::string::npos) return texto;
    const auto corte = texto.find_last_of(".?!");
    // Solo se recorta si queda al menos una frase completa
    return corte == std::string::npos ? texto : texto.substr(0, corte + 1);
}

}  // namespace jarvis
