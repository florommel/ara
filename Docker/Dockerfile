from ubuntu:focal
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Berlin

RUN apt update\
	&& apt-get install -y pkg-config clang-9 llvm-9-dev python3 python3-pip tmux build-essential ninja-build git binutils-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib bash-completion gdb-multiarch
RUN echo "deb http://downloads.skewed.de/apt focal main" > /etc/apt/sources.list.d/graph_tool.list\
	&& apt-key adv --keyserver keys.openpgp.org --recv-key 612DEFB798507F25\
	&& apt-get update \
	&& apt-get install -y python3-graph-tool
RUN apt-get install -y libboost-graph-dev cython3 python3-pydot python3-pydotplus\
	libsparsehash-dev gperf cmake g++-multilib\
	libboost-python-dev
	#python-pydot python-pydotplus cython #should not be needed since we're writing python3 code

# dependencies for qemu-stm32
RUN apt-get install -y libnss3 libglib2.0-dev libpixman-1-dev libfdt-dev libtinfo5

# tool for stm32 nucleo board flashing
RUN apt-get install -y stlink-tools

# get newer versions since the currently available ones dont fulfill or requirements
# we need 0.53.0 or >= 0.55.0
RUN pip3 install meson>=0.54.2 cython

RUN apt-get install -y python3-serial python3-future python3-matplotlib
RUN pip3 install -U versuchung matplotlib
#RUN apt-get install make unrar-free autoconf automake libtool gcc g++ gperf flex bison texinfo gawk ncurses-dev libexpat-dev python-dev python python-serialsed git unzip bash help2man wget bzip2 libtool-bin
RUN apt-get install -y strace


CMD ["bash"]
