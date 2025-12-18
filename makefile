CXX ?= g++
CXXFLAGS = -std=c++17
LDFLAGS = -lpthread -lmysqlclient -lssl -luuid -lcrypto

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2
endif

SRCS = main/main.cpp timer/lst_timer.cpp http/http_conn.cpp http/request_way.cpp log/log.cpp CGImysql/sql_connection_pool.cpp webserver/webserver.cpp config/config.cpp base64/base64.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

server: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

parallel:
	make -j$(shell nproc) server

fast:
	$(CXX) -o server $(SRCS) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f server $(OBJS) $(DEPS)

.PHONY: clean parallel fast
