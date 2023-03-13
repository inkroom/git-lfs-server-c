FROM registry.gitlab.com/rust_musl_docker/image:stable-latest
ADD . /workdir
VOLUME /root/.cargo/git
VOLUME /root/.cargo/registry
RUN cargo build --release -vv --target=x86_64-unknown-linux-musl


FROM scratch
COPY --from=0 /workdir/target/x86_64-unknown-linux-musl/release/server /server
CMD ["/server"]
