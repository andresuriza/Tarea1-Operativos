FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential pkg-config iputils-ping netcat-traditional curl \
  && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY client.c Makefile ./
RUN make client

# IP fija hacia el host; el puerto lo pasás en runtime
ENTRYPOINT ["./client", "--ip", "host.docker.internal"]