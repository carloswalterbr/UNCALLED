#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "map_pool.hpp"

bool load_conf(int argc, char** argv, Conf &conf);

int main(int argc, char** argv) {
    std::cerr << "Loading conf\n";

    Conf conf;

    if (!load_conf(argc, argv, conf)) {
        return 1;
    }

    MapPool pool(conf);

    u64 MAX_SLEEP = 100;

    std::cerr << "Mapping\n";

    Timer t;

    while (pool.running()) {
        u64 t0 = t.get();
        for (Paf p : pool.update()) {
            p.print_paf();
        }
        u64 dt = t.get() - t0;
        if (dt < MAX_SLEEP) usleep(1000*(MAX_SLEEP - dt));
    }

    std::cerr << "Finishing\n";

    pool.stop();

}

#define FLAG_TO_CONF(C, T, F) { \
    case C: \
        conf.set_##F(T(optarg)); \
        break; \
}

#define POSITIONAL_TO_CONF(T, F) {\
    if (i < argc) { \
        conf.set_##F(T(argv[i])); \
        i++; \
    } else { \
        std::cerr << "Error: must specify " << #F << "\n"; \
        return false; \
    } \
}

bool load_conf(int argc, char** argv, Conf &conf) {
    int opt;
    std::string flagstr = ":t:n:l:";

    #ifdef DEBUG_OUT
    flagstr += "D:";
    #endif

    while((opt = getopt(argc, argv, flagstr.c_str())) != -1) {

        switch(opt) {  

            FLAG_TO_CONF('t', atoi, threads)
            FLAG_TO_CONF('n', atoi, max_reads)
            FLAG_TO_CONF('l', std::string, read_list)

            #ifdef DEBUG_OUT
            FLAG_TO_CONF('D', std::string, dbg_prefix);
            #endif

            case ':':  
            std::cerr << "Error: failed to load flag value\n";  
            return false;

            case '?':  
            std::cerr << "Error: unknown flag\n";  
            return false;
        }
    }


    //parse positionals
    int i = optind;

    POSITIONAL_TO_CONF(std::string, bwa_prefix)
    POSITIONAL_TO_CONF(std::string, fast5_list)

    return true;
}
