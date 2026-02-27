# Zoomfolder

Open-source clone of [ZoomDir](https://www.arteryengine.com/zoomdir/) by Viktor Chlumský.

Desktop disk space visualizer — top-down flamegraph with real-time progressive scanning.

## Dev

Prerequisites: `cmake`, `ninja`, Xcode Command Line Tools.

```bash
brew install cmake ninja
```

Build and run:

```bash
make run
```

Run tests:

```bash
make test
```

Build in Docker (Linux, no local deps):

```bash
make docker
```
