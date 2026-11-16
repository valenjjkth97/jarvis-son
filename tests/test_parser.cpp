// Prueba unitaria del extractor de respuestas (se ejecuta en CI)
#include <cassert>
#include <iostream>

#include "jarvis.hpp"

int main() {
    const std::string salida =
        "Loading model...\n\x1b[1mllama.cpp\x1b[0m\nbuild : b1\navailable commands:\n  /glob <pattern>  add\n\n"
        "> Cuál es la capital de Colombia\nLa capital de Colombia es Bogotá.\n\n"
        "[ Prompt: 101.3 t/s | Generation: 19.8 t/s ]\n\nExiting...\n";
    const auto r = jarvis::extraer_respuesta(salida);
    assert(r.texto == "La capital de Colombia es Bogotá.");
    assert(r.tokens_s > 19.7 && r.tokens_s < 19.9);
    assert(jarvis::limpiar_transcripcion("  Hola Jarvis.\n\n") == "Hola Jarvis.");
    assert(jarvis::extraer_respuesta("").texto.empty());
    const auto r2 = jarvis::extraer_respuesta("> P\n**1.** Hola </think> **Bogotá** es la capital.\n[ Prompt: 1 t/s ]\n");
    assert(r2.texto == "Bogotá es la capital.");
    assert(jarvis::quitar_emojis("Hola \xF0\x9F\x98\x8A Jarvis \xE2\x98\x95\xEF\xB8\x8F") == "Hola  Jarvis ");
    assert(jarvis::quitar_emojis("Bogotá, ¿café?") == "Bogotá, ¿café?");
    const auto r3 = jarvis::extraer_respuesta("> P\nEl café ayuda. \xF0\x9F\x98\x8A\xE2\x98\x95\n[ Prompt: 1 t/s ]\n");
    assert(r3.texto == "El café ayuda.");
    assert(jarvis::limpiar_transcripcion(" [Música]\n").empty());
    assert(jarvis::limpiar_transcripcion("[MÚSICA]").empty());
    assert(jarvis::limpiar_transcripcion("¿Qué es una red neuronal?") == "¿Qué es una red neuronal?");
    assert(jarvis::recortar_frase_incompleta("Hola. Es importante usar el modo exces") == "Hola.");
    assert(jarvis::recortar_frase_incompleta("Solo una frase cortada") == "Solo una frase cortada");
    assert(jarvis::recortar_frase_incompleta("Bogotá.") == "Bogotá.");
    std::cout << "test_parser: OK\n";
    return 0;
}
