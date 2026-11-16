#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>

#include "jarvis.hpp"

namespace jarvis {

ResultadoProceso ejecutar(const std::vector<std::string>& argv, bool capturar_stdout) {
    ResultadoProceso r;
    if (argv.empty()) return r;
    const auto inicio = std::chrono::steady_clock::now();

    int tubo[2] = {-1, -1};
    if (capturar_stdout && pipe(tubo) != 0) return r;

    const pid_t pid = fork();
    if (pid < 0) return r;
    if (pid == 0) {  // proceso hijo
        const int nulo = open("/dev/null", O_RDWR);
        dup2(nulo, STDIN_FILENO);   // ningún hijo lee del teclado
        dup2(nulo, STDERR_FILENO);  // diagnósticos fuera de la pantalla
        if (capturar_stdout) {
            dup2(tubo[1], STDOUT_FILENO);
            close(tubo[0]);
            close(tubo[1]);
        } else {
            dup2(nulo, STDOUT_FILENO);
        }
        std::vector<char*> args;
        for (const auto& a : argv) args.push_back(const_cast<char*>(a.c_str()));
        args.push_back(nullptr);
        execvp(args[0], args.data());
        _exit(127);  // no se encontró el programa
    }

    if (capturar_stdout) {
        close(tubo[1]);
        char buf[4096];
        ssize_t n;
        while ((n = read(tubo[0], buf, sizeof buf)) > 0) r.salida.append(buf, static_cast<size_t>(n));
        close(tubo[0]);
    }

    int estado = 0;
    struct rusage uso {};
    wait4(pid, &estado, 0, &uso);
    r.codigo = WIFEXITED(estado) ? WEXITSTATUS(estado) : -1;
    r.rss_max_kb = uso.ru_maxrss;  // en Linux/Android viene en KB
    r.ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - inicio).count();
    return r;
}

}  // namespace jarvis
