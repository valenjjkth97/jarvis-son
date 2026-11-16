// Jarvis-son V1 · orquestador por turnos
// LISTO -> GRABANDO -> STT -> SLM -> TTS -> LISTO (o ERROR -> LISTO)
// Cada etapa es un proceso local con argumentos fijos; termina antes de que empiece la siguiente.
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "jarvis.hpp"

using namespace jarvis;

namespace {

struct Turno {
    int numero = 0;
    double audio_s = 0;
    long stt_ms = 0, slm_ms = 0, tts_ms = 0, total_ms = 0;
    long stt_kb = 0, slm_kb = 0, tts_kb = 0;
    double tokens_s = 0;
    std::string estado = "ok", etapa_error;
};

void mostrar(Estado e, const std::string& msg = "") {
    std::cout << "[" << nombre_estado(e) << "] " << msg << std::endl;
}

bool existe(const std::string& ruta) {
    struct stat st {};
    return stat(ruta.c_str(), &st) == 0;
}

long tamano(const std::string& ruta) {
    struct stat st {};
    return stat(ruta.c_str(), &st) == 0 ? static_cast<long>(st.st_size) : 0;
}

void borrar(const std::string& ruta) { std::remove(ruta.c_str()); }

std::string fecha_hora() {
    char buf[32];
    const std::time_t t = std::time(nullptr);
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

void registrar(const std::string& csv, const Turno& t) {
    const bool nuevo = tamano(csv) == 0;
    std::ofstream f(csv, std::ios::app);
    if (nuevo)
        f << "turno,fecha,audio_s,stt_ms,slm_ms,tts_ms,total_ms,stt_rss_kb,slm_rss_kb,tts_rss_kb,gen_tps,estado,"
             "etapa_error\n";
    // Privacidad: solo métricas, nunca el texto de la conversación
    f << t.numero << ',' << fecha_hora() << ',' << t.audio_s << ',' << t.stt_ms << ',' << t.slm_ms << ','
      << t.tts_ms << ',' << t.total_ms << ',' << t.stt_kb << ',' << t.slm_kb << ',' << t.tts_kb << ','
      << t.tokens_s << ',' << t.estado << ',' << t.etapa_error << '\n';
}

bool fallar(Turno& t, const std::string& etapa, int codigo, const std::string& pista) {
    t.estado = "error";
    t.etapa_error = etapa;
    mostrar(Estado::Error, "etapa=" + etapa + " codigo_salida=" + std::to_string(codigo) + " -> " + pista);
    return false;
}

// GRABANDO: Termux:API graba y FFmpeg normaliza a WAV mono 16 kHz
bool grabar(const Config& c, const std::string& crudo, const std::string& wav, Turno& t) {
    const int seg = c.get_int("segundos_grabacion", 6);
    ejecutar({"termux-microphone-record", "-q"}, false);
    auto r = ejecutar({"termux-microphone-record", "-f", crudo, "-l", std::to_string(seg)}, false);
    if (r.codigo != 0) return fallar(t, "grabacion", r.codigo, "revisa Termux:API y el permiso de micrófono");
    // El aviso sale cuando la grabación ya empezó, para no cortar el inicio de la frase
    mostrar(Estado::Grabando, "Habla ahora (" + std::to_string(seg) + " s)...");
    std::this_thread::sleep_for(std::chrono::seconds(seg + 1));
    ejecutar({"termux-microphone-record", "-q"}, false);
    if (tamano(crudo) == 0) return fallar(t, "grabacion", 0, "no se creó el audio");
    r = ejecutar({"ffmpeg", "-nostdin", "-loglevel", "error", "-y", "-i", crudo, "-ac", "1", "-ar", "16000", "-c:a",
                  "pcm_s16le", wav},
                 false);
    borrar(crudo);
    if (r.codigo != 0) return fallar(t, "normalizacion", r.codigo, "ffmpeg no pudo convertir el audio");
    t.audio_s = static_cast<double>(tamano(wav) - 44) / 32000.0;  // 16 kHz * 2 bytes
    return true;
}

// TTS: sherpa-onnx (voz Piper local) o, como respaldo de demostración, termux-tts-speak
bool hablar(const Config& c, const std::string& texto, const std::string& wav_tts, Turno& t) {
    mostrar(Estado::Hablando, texto);
    if (c.get("tts_modo", "termux") == "sherpa") {
        auto r = ejecutar({c.get("sherpa_bin"), "--vits-model=" + c.get("sherpa_modelo"),
                           "--vits-tokens=" + c.get("sherpa_tokens"), "--vits-data-dir=" + c.get("sherpa_datos"),
                           "--num-threads=4", "--output-filename=" + wav_tts, texto},
                          false);
        t.tts_ms = r.ms;
        t.tts_kb = r.rss_max_kb;
        if (r.codigo != 0 || !existe(wav_tts)) return fallar(t, "tts", r.codigo, "sherpa-onnx no generó el audio");
        const double dur = static_cast<double>(tamano(wav_tts) - 44) / (2.0 * c.get_int("sherpa_hz", 22050));
        ejecutar({"termux-media-player", "play", wav_tts}, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(dur * 1000) + 400));
        borrar(wav_tts);
        return true;
    }
    auto r = ejecutar({"termux-tts-speak", "-l", "es", texto}, false);
    t.tts_ms = r.ms;
    t.tts_kb = r.rss_max_kb;
    if (r.codigo != 0) return fallar(t, "tts", r.codigo, "termux-tts-speak falló (¿voz en español instalada?)");
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    std::string ruta_conf = "config/jarvis.conf";
    bool modo_texto = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--texto") modo_texto = true;  // escribir la pregunta (para depurar SLM + TTS)
        else if (a == "--config" && i + 1 < argc) ruta_conf = argv[++i];
        else {
            std::cout << "Uso: jarvis [--config archivo] [--texto]\n";
            return a == "--help" ? 0 : 2;
        }
    }

    const Config c = cargar_config(ruta_conf);
    // Las bibliotecas compartidas de sherpa-onnx viven en su carpeta de compilación
    if (!c.get("sherpa_lib").empty()) {
        const char* previo = std::getenv("LD_LIBRARY_PATH");
        const std::string ruta = c.get("sherpa_lib") + (previo ? std::string(":") + previo : "");
        setenv("LD_LIBRARY_PATH", ruta.c_str(), 1);
    }
    const std::string tmp = c.get("dir_temporal", "tmp");
    const std::string csv = c.get("registro_csv", "logs/turnos.csv");
    mkdir(tmp.c_str(), 0700);
    mkdir(c.get("dir_logs", "logs").c_str(), 0700);
    const std::string crudo = tmp + "/turno.m4a", wav = tmp + "/turno.wav", wav_tts = tmp + "/respuesta.wav";

    for (const auto& clave : {"whisper_cli", "whisper_modelo", "llama_cli", "llama_modelo"}) {
        if (!existe(c.get(clave))) {
            std::cerr << "Falta " << clave << " = " << c.get(clave) << " (revisa " << ruta_conf << ")\n";
            return 1;
        }
    }

    std::cout << "Jarvis-son V1 · modo " << (modo_texto ? "texto" : "voz") << " · registro en " << csv << "\n";
    int numero = 0;
    const bool memoria = c.get_int("memoria_turno_anterior", 1) == 1;
    std::string pregunta_previa, respuesta_previa;  // solo en memoria; no se guardan en disco
    while (true) {
        mostrar(Estado::Listo, modo_texto ? "Escribe tu pregunta (q para salir):"
                                          : "Presiona Enter para hablar (q + Enter para salir)");
        std::string entrada;
        if (!std::getline(std::cin, entrada) || entrada == "q") break;

        Turno t;
        t.numero = ++numero;
        const auto inicio = std::chrono::steady_clock::now();
        std::string pregunta;
        bool ok = true;

        if (modo_texto) {
            pregunta = entrada;
        } else {
            ok = grabar(c, crudo, wav, t);
            if (ok) {
                mostrar(Estado::Transcribiendo, "whisper.cpp...");
                auto r = ejecutar({c.get("whisper_cli"), "-m", c.get("whisper_modelo"), "-f", wav, "-l", "es", "-t",
                                   c.get("hilos", "4"), "-np", "-nt"});
                t.stt_ms = r.ms;
                t.stt_kb = r.rss_max_kb;
                borrar(wav);
                if (r.codigo != 0) ok = fallar(t, "stt", r.codigo, "whisper-cli falló");
                else pregunta = limpiar_transcripcion(r.salida);
            }
            if (ok && (pregunta.empty() || pregunta.find("BLANK_AUDIO") != std::string::npos)) {
                mostrar(Estado::Transcribiendo, "No te escuché bien. Intenta de nuevo.");
                t.estado = "sin_voz";
                ok = false;
            }
            if (ok) std::cout << "   Tú: " << pregunta << std::endl;
        }

        std::string respuesta;
        std::string entrada_slm = pregunta;
        if (ok && memoria && !pregunta_previa.empty())
            entrada_slm = "Turno anterior. Usuario: " + pregunta_previa + " | Jarvis: " + respuesta_previa +
                          " | Pregunta actual: " + pregunta;
        if (ok) {
            mostrar(Estado::Generando, "Qwen pensando...");
            std::vector<std::string> args = {c.get("llama_cli"), "-m", c.get("llama_modelo"), "-c",
                                             c.get("contexto", "1024"), "-n", c.get("max_tokens", "96"), "-t",
                                             c.get("hilos", "4"), "-sys", c.get("instruccion_sistema"), "-p", entrada_slm,
                                             "-st", "--reasoning-budget", "0", "--simple-io"};
            if (!c.get("razonamiento").empty()) args.insert(args.end(), {"-rea", c.get("razonamiento")});
            if (!c.get("penalizacion_repeticion").empty())
                args.insert(args.end(), {"--repeat-penalty", c.get("penalizacion_repeticion")});
            auto r = ejecutar(args);
            t.slm_ms = r.ms;
            t.slm_kb = r.rss_max_kb;
            const auto rs = extraer_respuesta(r.salida);
            t.tokens_s = rs.tokens_s;
            respuesta = rs.texto;
            if (r.codigo != 0 && respuesta.empty()) ok = fallar(t, "slm", r.codigo, "llama-cli falló");
            else if (respuesta.empty()) ok = fallar(t, "slm", r.codigo, "respuesta vacía");
        }

        if (ok) {
            hablar(c, respuesta, wav_tts, t);
            pregunta_previa = pregunta;
            respuesta_previa = respuesta;
        }

        borrar(crudo);
        borrar(wav);
        borrar(wav_tts);
        t.total_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - inicio).count();
        registrar(csv, t);
        std::cout << "   tiempos: STT " << t.stt_ms << " ms | SLM " << t.slm_ms << " ms | TTS " << t.tts_ms
                  << " ms | respuesta " << (t.stt_ms + t.slm_ms + t.tts_ms) << " ms | total " << t.total_ms << " ms | RAM pico SLM " << t.slm_kb / 1024 << " MB\n";
    }
    std::cout << "Hasta luego.\n";
    return 0;
}
