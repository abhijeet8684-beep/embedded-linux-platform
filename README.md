# Embedded Linux Platform

This repository contains the project-owned material for a portfolio-quality
Embedded Linux platform built with Yocto/Poky.

## Repository boundary

Poky is an external dependency and is not copied into this repository. The
active Poky checkout is expected at:

`~/Projects/Embedded/Yocto/poky`

The `meta-embedded-lab` layer contains the current project sources, recipes,
device-tree input, userspace tooling, tests, and architecture documentation.

## Current status

This repository was separated from the official Poky checkout before further
implementation work. The layer is under validation; no project commit or
remote has been created yet.

See [docs/environment-baseline.md](docs/environment-baseline.md) and
[meta-embedded-lab/docs/architecture.md](meta-embedded-lab/docs/architecture.md)
for the current environment and design notes.
