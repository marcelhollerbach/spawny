# automatic building on push is not activated (i expect the dockerfile to change rarely)
FROM archlinux/base
MAINTAINER Marcel Hollerbach

RUN pacman -Syy
RUN pacman -S base-devel git meson ninja protobuf-c check --noconfirm

RUN useradd tester
RUN useradd spawny
RUN echo 'tester:tester-password' | chpasswd
RUN mkdir /var/lib/spawny
RUN usermod -d /var/lib/spawny spawny
