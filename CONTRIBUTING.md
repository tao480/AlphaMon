# Contributing to AlphaMon Technical Assets

Thank you for your interest in contributing to The AlphaMon Platform! Your contributions help make the "open architecture" vision a reality.

## 🛠 What Can You Contribute?

We are looking for technical assets that help other developers and power users:
*   **Device Driver Files (DDFs)**: JSON files for new sensors or peripherals.
*   **Configuration Files**: Optimized configurations for specific use cases.
*   **Documentation**: Corrections or "Tips & Tricks" for using AlphaMon hardware.
*   **Bug Reports**: Identifying issues in existing templates or documentation.

## 📝 Contribution Process

1.  **Fork the Repository**: Create your own copy of this project.
2.  **Create a Branch**: Work on your changes in a dedicated branch.
3.  **Submit a Pull Request (PR)**:
    *   Clearly describe what your DDF or Config does.
    *   Ensure your JSON files are valid and follow the DDF standards.
    *   If adding a new hardware feature, include any relevant pinout information.

## 📐 Standards

### Device Driver Files (JSON)
*   **Validation**: Must be valid JSON.
*   **Comments**: Embeded comments are NOT supported.
*   **Naming**: Use uppercase (max 8 characters) followed by the firmware version the DDF is designed to support. e.g. `.417'.

### Configuration Files (Text)
*   **Clarity**: Include comments lines starting with `#` or add trailing comments using backslash pairs `//`.
*   **Safety**: Ensure that configurations do not conflict with the core AlphaMon hardware.
*   **Validation**: Being an IoT device, the AlphaMon doesn't have comprehensive validation checks, so draft with care!

## ⚖️ Licensing

By contributing to this repository, you agree that your contributions will be licensed under the project's [MIT License](./LICENSE). 

---
*Questions? Open an issue or contact the maintainers via the AlphaMon website.*
