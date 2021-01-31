FROM ubuntu

LABEL description="Container to build logalizer"

RUN apt-get update && apt-get install -y \
    g++ cmake libncurses5-dev libgtest-dev
