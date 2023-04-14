FROM devkitpro/devkita64 as base

ENV DEVKITPRO /opt/devkitpro
ENV DEVKITA64 ${DEVKITPRO}/devkitA64
ENV DEVKITARM ${DEVKITPRO}/devkitARM
ENV DEVKITPPC ${DEVKITPRO}/devkitPPC
ENV PORTLIBS_PREFIX ${DEVKITPRO}/portlibs/switch

ENV PATH ${DEVKITPRO}/tools/bin:$PATH
ENV PATH ${DEVKITA64}/bin/:$PATH

ENV WORK_DIR /home/work
RUN mkdir -p $WORK_DIR
WORKDIR $WORK_DIR

ENV REPO_DIR $WORK_DIR/repo
ENV BUILD_DIR $WORK_DIR/build
ENV INSTALL_DIR $WORK_DIR/install

RUN mkdir -p $REPO_DIR
RUN mkdir -p $BUILD_DIR
RUN mkdir -p $INSTALL_DIR

SHELL ["/bin/bash", "-c"]

RUN mkdir -p $REPO_DIR/SimpleModManager
RUN mkdir -p $BUILD_DIR/SimpleModManager
COPY . $REPO_DIR/SimpleModManager

RUN cd $REPO_DIR/SimpleModManager && \
    git submodule update --init --recursive && \
    cd $BUILD_DIR/SimpleModManager && \
    # for some reason yaml-cpp in not found by cmake, so put the paths manually
    cmake \
      -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR \
      -D CMAKE_TOOLCHAIN_FILE=$REPO_DIR/SimpleModManager/devkita64-libnx.cmake \
      $REPO_DIR/SimpleModManager && \
    make -j3 install
