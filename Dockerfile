FROM fedora:41 AS deps

# Install build tools + Qt6 + KF6
RUN dnf install -y \
    cmake \
    ninja-build \
    gcc-c++ \
    git \
    gettext \
    qt6-qtbase-devel \
    qt6-qttools-devel \
    extra-cmake-modules \
    kf6-karchive-devel \
    kf6-kcompletion-devel \
    kf6-kconfig-devel \
    kf6-kconfigwidgets-devel \
    kf6-kcoreaddons-devel \
    kf6-kcrash-devel \
    kf6-kdoctools-devel \
    kf6-ki18n-devel \
    kf6-kio-devel \
    kf6-kxmlgui-devel \
    && dnf clean all

FROM deps AS build

WORKDIR /src
COPY . /src

RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=/usr

RUN cmake --build build
