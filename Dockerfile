FROM ghcr.io/inkroomtemp/rust_musl_build:1.69.0
RUN  cargo new app
COPY Cargo.toml /workdir/app
COPY build.rs /workdir/app
RUN cd /workdir/app && cargo build --release -vv --target=x86_64-unknown-linux-musl --all-features
VOLUME /root/.cargo/git
VOLUME /root/.cargo/registry
ARG CARGO_FEATURES
COPY . /workdir/app
RUN cd /workdir/app && ((test -e Cargo-docker.toml &&  rm -rf Cargo.toml && mv Cargo-docker.toml Cargo.toml ) || rm -rf /workdir/app/target/x86_64-unknown-linux-musl/release/deps/server-* ) \
  && cargo build --release -vv --target=x86_64-unknown-linux-musl ${CARGO_FEATURES} \
  && ./target/x86_64-unknown-linux-musl/release/server -v


# FROM alpine
# RUN apk add ca-certificates
FROM scratch
COPY --from=0 /etc/ssl /etc/ssl
COPY --from=0 /usr/share/ca-certificates /usr/share/ca-certificates
COPY --from=0 /workdir/app/target/x86_64-unknown-linux-musl/release/server /server
CMD ["/server"]
