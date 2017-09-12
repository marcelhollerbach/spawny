# automatic building on push is not activated (i expect the dockerfile to change rarely)
FROM dock0/arch
MAINTAINER Leander Kurscheidt

RUN pacman -Syy
RUN pacman -S base-devel git openssh cmake meson ninja protobuf-c check --noconfirm
