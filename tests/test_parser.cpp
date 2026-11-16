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
    std::cout << "test_parser: OK\n";
    return 0;
}
