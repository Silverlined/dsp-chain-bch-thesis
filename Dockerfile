FROM ubuntu:20.04
MAINTAINER dimitriy.georgiev@endurosat.com

RUN apt-get update -q && DEBIAN_FRONTEND=noninteractive \
       apt-get install -qy \
         gnuradio gr-osmosdr gr-limesdr \
         librtlsdr-dev rtl-sdr  soapysdr-module-rtlsdr soapysdr-module-lms7\
         gr-iio  gr-iqbal \
         uhd-host uhd-soapysdr avahi-daemon libuhd-dev python3-uhd python3-paho-mqtt\
         limesuite limesuite-udev \
         soapysdr-tools python3-soapysdr \
         nano screen sudo build-essential cmake gnuradio-dev libssl-dev git swig liborc-0.4-dev vim\
         --no-install-recommends \
         && apt-get -qq clean all

### for source building 
RUN apt-get install -qy \
        build-essential cmake gnuradio-dev \
        libssl-dev git swig liborc-0.4-dev \
        vim python3-pip
#sudo service dbus start
#service avahi-daemon start 

### gr-satnogs deps
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get install -qy \
    libpng++-dev \
    nlohmann-json3-dev \
    libvorbis-dev \
    libitpp-dev \
    liborc-0.4-0 \
    liborc-0.4-dev \
    liborc-0.4-dev-bin \
    libitpp-dev \
    libitpp8v5

RUN pip3 install construct
 
### Create a dedicated user (not necessary)
#RUN useradd -ms /bin/bash radio && echo "radio:radio" | chpasswd && adduser radio sudo
#RUN usermod -a -G dialout radio
#RUN echo '%sudo	ALL=(ALL:ALL) ALL' >> /etc/sudoers
#RUN echo 'radio ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
#USER radio

WORKDIR /home/radio
RUN mkdir -p /home/radio/gr-oot/gr-satnogs/build /home/radio/gr-oot/gr-satellites/build /home/radio/gr-oot/gr-si446x/build /home/radio/gr-oot/gr-iocp_client/build /home/radio/gr-grc
COPY gr-grc/* /home/radio/gr-grc

### Build GR-OOT from source
## GR-SATNOGS
COPY gr-oot/gr-satnogs /home/radio/gr-oot/gr-satnogs
WORKDIR /home/radio/gr-oot/gr-satnogs/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j4 \
    && make install
## GR-SATELLITES
COPY gr-oot/gr-satellites /home/radio/gr-oot/gr-satellites
WORKDIR /home/radio/gr-oot/gr-satellites/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j4 \
    && make install
## GR-SI446X
COPY gr-oot/gr-si446x /home/radio/gr-oot/gr-si446x
WORKDIR /home/radio/gr-oot/gr-si446x/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j4 \
    && make install
## GR-IOCP_CLIENT
COPY gr-oot/gr-iocp_client /home/radio/gr-oot/gr-iocp_client
WORKDIR /home/radio/gr-oot/gr-iocp_client/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j4 \
    && sudo make install

RUN sudo ldconfig

WORKDIR /home/radio/gr-grc
ENV PYTHONPATH "${PYTHONPATH}:/usr/local/lib/python3/dist-packages"

#CMD ["/bin/bash", "--login"]
#RUN echo "Start"
