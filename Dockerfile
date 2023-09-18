FROM ubuntu:20.04

RUN apt -y update && apt install -y wget g++ unzip zip
RUN apt-get install xz-utils
RUN wget https://github.com/bazelbuild/bazel/releases/download/6.3.0/bazel-6.3.0-installer-linux-x86_64.sh
RUN chmod 755 bazel-6.3.0-installer-linux-x86_64.sh
RUN ./bazel-6.3.0-installer-linux-x86_64.sh
RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz
RUN tar -xf clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz 
RUN ln -s $(pwd)/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04 /usr/local/llvm
RUN echo "LLVM_HOME=/usr/local/llvm" >> ~/.bashrc
ENV PATH="${PATH}:/usr/local/llvm/bin"
echo "Build Done"
