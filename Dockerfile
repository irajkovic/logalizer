FROM ubuntu

LABEL description="Container to build logalizer"

# Prevent tzdata for asking about region to avoid freeze
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ cmake libncurses5-dev libgtest-dev lcov
