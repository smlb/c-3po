FROM alpine:latest

RUN apk update && \
    apk add --no-cache gcc make musl-dev curl-dev  

RUN mkdir /bot
ADD . /bot
WORKDIR /bot

RUN make 

WORKDIR /bot/bin
CMD ./c-3po irc.freenode.net 6667 bottanbot \#\#sysyphus porcodio 
