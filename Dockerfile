FROM ubuntu:20.04
ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && export DEBIAN_FRONTEND=noninteractive
# libapr1-dev
RUN sed -i "s@http://.*archive.ubuntu.com@http://mirrors.huaweicloud.com@g" /etc/apt/sources.list \
    && sed -i "s@http://.*security.ubuntu.com@http://mirrors.huaweicloud.com@g" /etc/apt/sources.list \
    && apt update -y && apt upgrade -y
RUN apt install -y cmake g++  libaprutil1-dev libcurl4-openssl-dev curl wget git libssl-dev
RUN wget https://github.com/michaelrsweet/mxml/releases/download/v3.3.1/mxml-3.3.1.tar.gz \
    && tar -zxf mxml-3.3.1.tar.gz && cd mxml-3.3.1 && ./configure  && make && make install
RUN wget https://dlcdn.apache.org/apr/apr-1.7.2.tar.gz \
    && tar -zxf apr-1.7.2.tar.gz && cd apr-1.7.2 && ./configure  && make && make install
RUN wget https://github.com/tencentyun/cos-c-sdk-v5/archive/refs/tags/v5.0.16.tar.gz \
    && tar -zxf v5.0.16.tar.gz && cd cos-c-sdk-v5-5.0.16 && cmake .  && make && make install
RUN wget https://curl.se/download/curl-7.88.1.tar.gz
RUN apt install -y libssl-dev
RUN tar -zxf curl-7.88.1.tar.gz && cd curl-7.88.1/ && ./configure --disable-ldap --disable-ldaps --with-openssl && make && make install

ADD . /data
RUN cd /data && git submodule update --init --recursive && mkdir /data/build && cd /data/build && cmake .. && make && chmod +x lfs && cp ../lfs.sh ./ && chmod +x lfs.sh  && cp ../pack.sh ./  && mkdir lib && sh pack.sh  
RUN 





FROM ubuntu:20.04
COPY --from=0  /data/build/ /lfs
EXPOSE 3000
ENTRYPOINT ["/lfs/entrypoint.sh"]