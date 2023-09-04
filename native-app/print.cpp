#include "print.h"

namespace lyniat::socket::print {
    fmt::color get_color(console_output_t type) {
        switch (type) {
            case PRINT_LOG:
                return fmt::color::gray;
            case PRINT_WARNING:
                return fmt::color::orange;
            case PRINT_ERROR:
                return fmt::color::red;
            default:
                return fmt::color::gray;
        }
    }
}