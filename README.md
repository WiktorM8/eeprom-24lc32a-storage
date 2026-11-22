# eeprom-24lc32a-storage

Aplikacja w języku C++ (C++20) na **Raspberry Pi 5** do obsługi zewnętrznej pamięci EEPROM **24LC32A** (4 KB) przez magistralę **I2C** oraz sprzętowej ochrony przed zapisem (**WP**) za pomocą biblioteki **libgpiod v2**.

## Podłączenie sprzętowe (Raspberry Pi 5)

| Pin fizyczny | Funkcja / GPIO | Sygnał 24LC32A | Uwagi |
| :--- | :--- | :--- | :--- |
| **1** | `3.3V` | `VCC` | Zasilanie (dioda sygnalizacyjna przez rezystor 220 $\Omega$) |
| **3** | `GPIO2` (`SDA`) | `SDA` | Magistrala `/dev/i2c-1`, adres układu `0x50` |
| **5** | `GPIO3` (`SCL`) | `SCL` | Magistrala `/dev/i2c-1` |
| **9** | `GND` | `GND`, `A0`, `A1`, `A2` | Masa |
| **11** | `GPIO17` | `WP` | Write Protect (`/dev/gpiochip0`, dioda stanu przez rezystor 220 $\Omega$) |

> **Uwaga:** Pin fizyczny **7 (`GPIO4`)** należy pozostawić wolny — jest on zarezerwowany przez obsługę przycisku zasilania obudowy Argon.

---

## Wymagane zależności na Raspberry Pi 5

1. **Instalacja kompilatora, CMake oraz bibliotek GPIO/I2C:**
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake gpiod libgpiod-dev i2c-tools
   ```

2. **Włączenie interfejsu I2C** (jeśli nie jest jeszcze aktywny):
   ```bash
   sudo raspi-config
   # Interface Options -> I2C -> Enable
   ```

3. **Uprawnienia użytkownika do magistrali I2C i GPIO** (wymagane ponowne zalogowanie):
   ```bash
   sudo usermod -aG i2c,gpio $USER
   ```

4. **Szybka weryfikacja sprzętowa (opcjonalnie):**
   ```bash
   i2cdetect -y 1   # układ 24LC32A powinien być widoczny pod adresem 0x50
   gpiodetect       # kontroler pinctrl-rp1 pod gpiochip0
   ```

---

## Kompilacja i uruchomienie

### 1. Zdalnie z systemu Windows (zalecane — przez `Taskfile`)

Wymaga zainstalowanego narzędzia [Task](https://taskfile.dev/) oraz skonfigurowanego dostępu SSH (najlepiej z kluczem SSH) do Raspberry Pi 5.

```powershell
# Synchronizacja plików, kompilacja i uruchomienie na RPi 5
task run

# Pozostałe dostępne zadania:
task sync    # przesłanie plików projektu na RPi 5
task build   # synchronizacja i kompilacja CMake na RPi 5
task clean   # usunięcie katalogu build na RPi 5 i lokalnego logu
task docs    # kompilacja dokumentacji LaTeX w katalogu docs/
```

Domyślny host to `wiktor@skynet.local`. Można go nadpisać przy wywołaniu:
```powershell
task run RPI_HOST=uzytkownik@adres_ip
```

### 2. Lokalnie bezpośrednio na Raspberry Pi 5

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
./eeprom_app
```

Opcjonalnie można przekazać własną ścieżkę magistrali oraz adres I2C:
```bash
./eeprom_app /dev/i2c-1 0x50
```