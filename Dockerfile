FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y \
    cmake ninja-build gcc g++ git pkg-config \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev \
    libxi-dev libxfixes-dev libxss-dev libwayland-dev \
    libxkbcommon-dev libasound2-dev libpulse-dev \
    libdbus-1-dev libdrm-dev libgbm-dev libegl-dev \
    libgl-dev libfreetype-dev libharfbuzz-dev libxtst-dev \
    libgtk-3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -B build -G Ninja && cmake --build build

RUN cd build && ctest --output-on-failure

FROM scratch AS export
COPY --from=build /src/build/zoomfolder /zoomfolder
