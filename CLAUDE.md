# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**elekter** is a C++17 command-line tool that processes CSV electricity consumption data from https://elering.ee and calculates costs using Nord Pool hourly spot prices. It uses Qt 6 (Core, Network, Sql), libfmt, and SQLite. User-facing messages and comments are in Estonian.

## Build Commands

```bash
# Configure (with vcpkg)
cmake -B bld -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake

# Configure (without vcpkg, system Qt/fmt)
cmake -B bld

# Build
cmake --build bld

# Install
cmake --install bld
```

No automated test suite exists. Manual testing uses `sample/tarbimisandmed.csv`.

## Linting

Clangd is configured via `.clangd` with ClangTidy checks (performance, modernize, readability, bugprone). The compilation database is expected at `bld/compile_commands.json`.

## Architecture

The app uses Qt's event loop (`QCoreApplication` subclass) with a single-shot timer to trigger processing.

**Key flow:** `main.cpp` → `App` (orchestrator) → loads `Consumption` (CSV parser) → fetches `Prices` (cache-first via `Cache`/SQLite, then `NordPool` API) → calculates and prints results.

**Components:**
- **Args** — singleton, parses CLI options via `getopt_long`
- **App** — main controller, calculates day/night totals, VAT, margin
- **Consumption** / **Record** / **Header** — CSV parsing with flexible Estonian header recognition, 15-min interval data
- **Prices** / **NordPool** / **JSON** — two-tier price fetching: SQLite cache (`~/.local/share/elekter/nordpool.db`) then Nord Pool API (`https://dashboard.elering.ee/api/nps/price`)
- **Cache** — SQLite storage with `blocks` and `prices` tables, transaction support
- **common.h** — shared types, `El::` namespace, custom `Exception` class, fmt formatters for Qt types

**Nord Pool API note:** Data switched from hourly to 15-minute intervals after 2025-10-01. Regions: ee, fi, lv, lt.

## Conventions

- All source files are in the project root (no src/ subdirectory)
- Qt automoc is enabled via CMake
- RAII and smart pointers throughout
- Custom fmt formatters bridge Qt types (QDateTime, QString) to libfmt
