FROM registry.gitlab.com/rust_musl_docker/image:stable-latest
RUN  cargo new app && ls && pwd
COPY Cargo.toml /workdir/app
COPY build.rs /workdir/app
RUN ls /workdir && cd /workdir/app && cargo build --release --target=x86_64-unknown-linux-musl --all-features
VOLUME /root/.cargo/git
VOLUME /root/.cargo/registry
ARG CARGO_FEATURES
COPY . /workdir/app
RUN cd /workdir/app && ((test -e Cargo-docker.toml &&  rm -rf Cargo.toml && mv Cargo-docker.toml Cargo.toml ) || true )  && cargo build --release --target=x86_64-unknown-linux-musl ${CARGO_FEATURES}


FROM scratch
COPY --from=0 /workdir/app/target/x86_64-unknown-linux-musl/release/server /server
CMD ["/server"]
