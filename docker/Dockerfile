FROM ubuntu

COPY . /spl/c

RUN apt-get update
RUN apt-get -y install locales
RUN ln -fs /usr/share/zoneinfo/Europe/Berlin /etc/localtime
# && dpkg-reconfigure -f noninteractive tzdata
# RUN sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && \
    # locale-gen
# ENV LANG en_US.UTF-8  
# ENV LANGUAGE en_US:en  
# ENV LC_ALL en_US.UTF-8   
RUN apt-get upgrade -y
RUN apt-get install python3 python3-pip flex bison cmake neovim -y
RUN /usr/bin/python3 -m pip install pipenv
RUN cp /spl/c/submit.sh /spl/
RUN cp -r /spl/c/.submitscript /spl/.submitscript

WORKDIR /spl
ENTRYPOINT ["./submit.sh"]
