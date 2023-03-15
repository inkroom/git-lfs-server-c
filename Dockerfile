FROM registry.gitlab.com/rust_musl_docker/image:stable-latest
ADD . /workdir
VOLUME /root/.cargo/git
VOLUME /root/.cargo/registry
ARG CARGO_FEATURES
RUN cargo build --release -vv --target=x86_64-unknown-linux-musl ${CARGO_FEATURES}


FROM scratch
COPY --from=0 /workdir/target/x86_64-unknown-linux-musl/release/server /server
CMD ["/server"]
