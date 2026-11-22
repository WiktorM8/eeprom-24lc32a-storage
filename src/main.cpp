#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <gpiod.h>

namespace {

bool probe_i2c_device(const std::string& dev_path, uint8_t addr, std::string& detail) {
    int fd = open(dev_path.c_str(), O_RDWR);
    if (fd < 0) {
        detail = "Nie udalo sie otworzyc " + dev_path + " (" + std::strerror(errno) + ")";
        return false;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        detail = std::string("Nie udalo sie ustawic adresu I2C (") + std::strerror(errno) + ")";
        close(fd);
        return false;
    }

    uint8_t dummy = 0;
    ssize_t ret = read(fd, &dummy, 1);
    int saved_errno = errno;
    close(fd);

    if (ret == 1) {
        detail = "Uklad odpowiedzial (ACK)";
        return true;
    }

    detail = std::string("Brak odpowiedzi (NACK / blad: ") + std::strerror(saved_errno) + ")";
    return false;
}

bool probe_gpio_wp(const char* chip_path, unsigned int wp_pin) {
    gpiod_chip* chip = gpiod_chip_open(chip_path);
    if (!chip) {
        std::cerr << "[GPIO] Nie mozna otworzyc " << chip_path << "\n";
        return false;
    }

    gpiod_line_settings* settings = gpiod_line_settings_new();
    gpiod_line_config* line_cfg = gpiod_line_config_new();
    gpiod_request_config* req_cfg = gpiod_request_config_new();

    if (!settings || !line_cfg || !req_cfg) {
        std::cerr << "[GPIO] Blad alokacji konfiguracji libgpiod\n";
        if (req_cfg) gpiod_request_config_free(req_cfg);
        if (line_cfg) gpiod_line_config_free(line_cfg);
        if (settings) gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return false;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);
    gpiod_line_config_add_line_settings(line_cfg, &wp_pin, 1, settings);
    gpiod_request_config_set_consumer(req_cfg, "eeprom_wp_test");

    gpiod_line_request* request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        std::cerr << "[GPIO] Nie mozna przejac linii GPIO" << wp_pin << "\n";
        gpiod_request_config_free(req_cfg);
        gpiod_line_config_free(line_cfg);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(chip);
        return false;
    }

    std::cout << "[GPIO] OK - " << chip_path << " linia GPIO" << wp_pin
              << " ustawiona jako wyjscie (WP=1)\n";

    gpiod_line_request_release(request);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    return true;
}

uint8_t parse_addr(const char* str) {
    char* end = nullptr;
    long val = std::strtol(str, &end, 0);
    if (end == str || *end != '\0' || val < 0x08 || val > 0x7f) {
        return 0;
    }
    return static_cast<uint8_t>(val);
}

} // namespace

int main(int argc, char* argv[]) {
    std::string dev_path = "/dev/i2c-1";
    uint8_t addr = 0x50;

    if (argc >= 2) {
        dev_path = argv[1];
    }
    if (argc >= 3) {
        addr = parse_addr(argv[2]);
        if (addr == 0) {
            std::cerr << "Niepoprawny adres I2C: " << argv[2] << "\n";
            return 2;
        }
    }

    std::string detail;
    bool found = probe_i2c_device(dev_path, addr, detail);
    probe_gpio_wp("/dev/gpiochip0", 17);

    std::cout << "[I2C] Magistrala: " << dev_path
              << " | Adres: 0x" << std::hex << static_cast<int>(addr) << std::dec
              << " | Status: " << (found ? "ZNALEZIONO" : "NIE ZNALEZIONO")
              << " (" << detail << ")\n";

    return found ? 0 : 1;
}