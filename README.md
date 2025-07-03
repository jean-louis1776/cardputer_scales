# Smart Scales for 🖥️ M5Cardputer ⚖️

**Author:** [@ilalex](https://github.com/jean-louis1776)

This project turns your **M5Cardputer** into a compact **digital smart scale** using the **HX711 load cell amplifier** and **M5Stack ecosystem**. Features include smooth weight display, persistent unit settings via SD card, and a simple on-device UI for unit selection and tare control.

---

## 🚀 Features

- 📦 **Plug & play support for HX711**
- 📊 Real-time, smoothed weight display
- ⚙️ Menu-based unit selection: grams (g), ounces (oz), pounds (lb)
- 💾 Configuration saved to SD card in JSON
- 🎵 Error notifications via speaker
- 🖥️ Clean UI with splash screen, weight view, error overlays

---

## 🧰 Hardware Requirements

- [M5Cardputer](https://shop.m5stack.com/products/m5cardputer)
- HX711 Load Cell Amplifier (e.g. from [Bogde's HX711 library](https://github.com/bogde/HX711))
- Load cell sensor
- microSD card (for saving settings)
- Jumper wires

---

## 🔌 Wiring

| Component | M5Cardputer Pin |
|----------:|:---------------:|
| HX711 `DT` (DOUT) | GPIO 33 |
| HX711 `SCK` (CLK) | GPIO 32 |
| VCC & GND | 3.3V / GND     |

---

## 🛠️ Libraries Used

Make sure to install the following libraries via the Arduino Library Manager:

- [M5Cardputer](https://github.com/m5stack/M5Cardputer)
- [HX711 by Bogde](https://github.com/bogde/HX711)
- [ArduinoJson](https://arduinojson.org/)
- [SD](https://www.arduino.cc/en/Reference/SD) (built-in)

---

## 🖥️ UI Controls

| Key | Action |
|-----|--------|
| `R` | Tare (reset scale to zero) |
| `S` / `Enter` | Open settings menu |
| `↑ / ↓` | Navigate menu |
| `Enter` | Confirm unit selection |

---

## ⚠️ Notes

- If the SD card is **missing or unreadable**, the settings will **not be saved** and a warning will be shown.
- If the scale is **not ready**, a blinking error overlay will appear.
- Weight values are **smoothed** for more stable display.

---

## 📷 Demo

> _Coming soon_ – GIF or video of the project in action

---

## 🙌 Credits

- Inspired by DIY digital scales and the awesome [HX711 Arduino library](https://github.com/bogde/HX711)
- Powered by [M5Stack](https://m5stack.com)

---

## 📜 License

MIT – use it, remix it, build something cool.

---

## 🧠 Future Ideas

- Calibration via menu
- Battery status overlay
- Bluetooth or Wi-Fi weight broadcasting
- Logging to SD

---

Enjoy building your own **pocket digital scale**!  
Made with ❤️ by @ilalex
