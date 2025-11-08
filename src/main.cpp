#include "application.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // !STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main() {
    const auto app = Application::getInstance();

    if (!app->initialize()) {
        return -1;
    }

    app->run();
    app->cleanup();

    return 0;
}
