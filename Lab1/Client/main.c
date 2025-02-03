#include "client.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        log_message(LOG_FILE, LOG_CRITICAL, "Server address not specified");
        exit(-1);
    }

    start_log(LOG_FILE);
    run(argv[1]);
    return 0;
}