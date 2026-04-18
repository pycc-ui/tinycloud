FROM ubuntu:22.04 AS builder

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libmysqlclient-dev \
    libssl-dev \
    uuid-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN make server

FROM ubuntu:22.04

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list

RUN apt-get update && apt-get install -y \
    libmysqlclient21 \
    libssl3 \
    uuid-runtime \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY --from=builder /build/server .

RUN addgroup --system --gid 1000 app && adduser --system --uid 1000 --gid 1000 app
RUN mkdir serverlog root && chown -R app:app /app

USER app

EXPOSE 9006 


CMD ["./server"]
