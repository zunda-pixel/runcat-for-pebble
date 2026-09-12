# RunCat for Pebble

A Pebble watch face where the official RunCat animation runs faster as your heart rate increases.

The primary target is Pebble Time 2. Watches without Health support continue to run the cat at a default speed.

## Building & running

```sh
pebble build                          # build for all targetPlatforms with the active SDK
pebble install --emulator emery       # install on the emery emulator
pebble install --phone <ip>           # install to a paired phone
```

The generated package is `build/runcat-watchface.pbw`.

## Releases

Pushing a Git tag runs the GitHub Actions release workflow. It builds the watchface with Pebble SDK 4.33.1, generates release notes from the commits since the previous release, and attaches `build/runcat-watchface.pbw` to the GitHub Release.

```sh
git tag v1.0.0
git push origin v1.0.0
```

This project is built with the latest active Pebble SDK. The watch must be updated to firmware 4.32 or later. Enable Pebble Health on the watch and in the companion app for BPM data.

```sh
pebble sdk activate 4.33.1
pebble build
```

## Features

- Current time and date
- RunCat bitmap animation
- Heart-rate-based animation speed
- Current heart rate display, with `-- BPM` fallback

## Credits

RunCat originally created by Takuto Nakamura (Kyome22).

Original RunCat: https://kyome.io/runcat/

RunCat Resources: https://github.com/runcat-dev/RunCatResources

The included RunCat resources are licensed under the Apache License, Version 2.0.
See `NOTICE` and `LICENSE` for attribution and license details.

## Target platforms

`targetPlatforms` in `package.json` controls which watches you build for. The
modern Pebble hardware is **emery** (Pebble Time 2), **gabbro** (Pebble Round
2), and **flint** (Pebble 2 Duo); the original Pebble platforms (aplite,
basalt, chalk, diorite) are included by default for backwards compatibility.

## Project layout

```
src/c/           C source for the watchapp
src/pkjs/        PebbleKit JS (phone-side) source, if any
worker_src/c/    Background worker source, if any
resources/       Images, fonts, and other bundled resources
package.json     Project metadata (UUID, platforms, resources, message keys)
wscript          Build rules — usually no need to edit
```

By default this project is configured as a watchapp. To make it a watchface,
set `pebble.watchapp.watchface` to `true` in `package.json`.

## Documentation

Full SDK docs, tutorials, and API reference: <https://developer.repebble.com>
