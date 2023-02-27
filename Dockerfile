FROM ubuntu:20.04
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && export DEBIAN_FRONTEND=noninteractive
# libapr1-dev
RUN sed -i "s@http://.*archive.ubuntu.com@http://mirrors.huaweicloud.com@g" /etc/apt/sources.list \
    && sed -i "s@http://.*security.ubuntu.com@http://mirrors.huaweicloud.com@g" /etc/apt/sources.list \
    && apt update -y && apt upgrade -y
RUN apt install -y cmake g++  libaprutil1-dev libcurl4-openssl-dev curl wget git libssl-dev libidn11-dev libc6-dev
RUN wget https://github.com/michaelrsweet/mxml/releases/download/v3.3.1/mxml-3.3.1.tar.gz \
    && tar -zxf mxml-3.3.1.tar.gz
RUN wget https://dlcdn.apache.org/apr/apr-1.7.2.tar.gz \
    && tar -zxf apr-1.7.2.tar.gz
RUN wget https://github.com/tencentyun/cos-c-sdk-v5/archive/refs/tags/v5.0.16.tar.gz \
    && tar -zxf v5.0.16.tar.gz
RUN wget https://curl.se/download/curl-7.88.1.tar.gz \
 && tar -zxf curl-7.88.1.tar.gz
RUN cd /mxml-3.3.1 && ./configure  && make && make install
RUN cd /apr-1.7.2 && ./configure  && make && make install
RUN cd /curl-7.88.1/ && ./configure --disable-ftp --disable-pop3 --disable-imap --disable-smb --disable-mqtt --disable-mime --disable-websockets --disable-debug --disable-curldebug --disable-rtsp --disable-telnet --disable-file --disable-smtp --disable-tftp --disable-gopher --disable-dict --disable-ldap --disable-ldaps --with-openssl && make && make install
RUN cd /cos-c-sdk-v5-5.0.16 && cmake .  && make && make install

ADD . /data
RUN cd /data && git submodule update --init --recursive && mkdir /data/build && cd /data/build && cmake .. -DUSE_STATIC_LINK=ON && make && chmod +x lfs





FROM busybox
COPY --from=0  /data/build/lfs /lfs/lfs
EXPOSE 3000
ENTRYPOINT ["/lfs/lfs"]