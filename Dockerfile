## --- Build Stage ---
FROM ubuntu:22.04 as builder

# Set environment variables to avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

ARG HOME=/home
ARG PACKAGE_NAME=video_stream
COPY include $HOME/build/$PACKAGE_NAME/include
COPY src $HOME/build/$PACKAGE_NAME/src
COPY CMakeLists.txt $HOME/build/$PACKAGE_NAME

RUN apt-get update \
    && apt-get install -y \
        build-essential \
        cmake \
        pkg-config \
        libopencv-dev \
    && mkdir -p $HOME/build/$PACKAGE_NAME/build \
    && cd $HOME/build/$PACKAGE_NAME/build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make \
    && rm -rf /var/lib/apt/lists/*


## --- Final Stage ---
# Use base image
FROM ubuntu:22.04

MAINTAINER Martin Økter <martino@uia.no>

# Set environment variables to avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

ARG HOME=/home
ARG PACKAGE_NAME_CONTAINER=eitri_video_streamer
ARG PACKAGE_NAME=video_stream

# set working dir for video
WORKDIR /app/$PACKAGE_NAME_CONTAINER

RUN apt-get update \
    && apt-get install -yqq alsa-utils ffmpeg libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

# copy files to docker
COPY --from=builder $HOME/build/$PACKAGE_NAME/build/video_stream /app/$PACKAGE_NAME_CONTAINER

# start stream
CMD [ "./video_stream" ]