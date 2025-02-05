FROM debian:bookworm AS build

# Enable source repos.
RUN sed -i '/Types: deb$/c\Types: deb deb-src' /etc/apt/sources.list.d/debian.sources

RUN apt update && \
    apt-get build-dep -y scribus && \
    apt install -y build-essential libqt6core5compat6-dev libqt6svg6-dev linguist-qt6 qt6-base-dev qt6-base-dev-tools qt6-base-private-dev qt6-gtk-platformtheme qt6-image-formats-plugins qt6-l10n-tools qt6-svg-dev qt6-tools-dev qt6-tools-dev-tools qt6-translations-l10n && \
    apt install -y zlib1g-dev libpng-dev libjpeg-dev libtiff5-dev libpython3-all-dev libfreetype6-dev libcairo2-dev libcups2-dev libxml++2.6-dev liblcms2-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app/src
ADD . /app/src

RUN cmake . -DCMAKE_INSTALL_PREFIX:PATH=/scribus
RUN make && make install

FROM debian:bookworm-slim

# Install xvfb and Scribus runtime dependencies.
RUN apt update && \
    apt install -y xvfb ghostscript libc6 libcairo2 libcdr-0.1-1 libcups2 libfontconfig1 libfreehand-0.1-1 libfreetype6 libgcc-s1 libgraphicsmagick-q16-3 libharfbuzz-icu0 libharfbuzz-subset0 libharfbuzz0b libhunspell-1.7-0 libicu72 libjpeg62-turbo liblcms2-2 libmspub-0.1-1 libopenscenegraph161 libopenthreads21 libpagemaker-0.0-0 libpng16-16 libpodofo0.9.8 libpoppler126 libpython3.11 libqt5core5a libqt5gui5 libqt5network5 libqt5opengl5 libqt5printsupport5 libqt5widgets5 libqt5xml5 libqxp-0.0-0 librevenge-0.0-0 libstdc++6 libtiff6 libvisio-0.1-1 libxml2 libzmf-0.0-0 zlib1g && \
    rm -rf /var/lib/apt/lists/*

COPY --from=build /scribus /scribus

ENV DEBIAN_FRONTEND noninteractive

