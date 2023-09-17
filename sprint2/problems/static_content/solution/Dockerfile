FROM gcc:11.3 as build

RUN apt update && \
	apt install -y \
	python3-pip \
	cmake \
	&& \
	pip3 install conan

COPY conanfilev2.txt /app/
RUN mkdir /app/build && cd /app/build && \
	conan profile detect && \
	conan install ../conanfilev2.txt -of .

COPY ./src /app/src
COPY CMakeLists.txt /app/

RUN cd /app/build && \
	cmake /app/ -G "Unix Makefiles" -DCONANV2=ON -DCMAKE_TOOLCHAIN_FILE=/app/build/conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Release && \
	cmake --build .

FROM ubuntu:22.04 as run

RUN groupadd -r www && useradd -r -g www www
USER www

COPY --from=build /app/build/game_server /app/
COPY ./data /app/data

ENTRYPOINT ["/app/game_server", "/app/data/config.json"]
