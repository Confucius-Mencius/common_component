CXX = gcc
CXXFLAGS += -g -Wall -Wextra

NGX_ROOT = /home/hgc/workspace/mine/c_third_party/archives/nginx-1.10.2

TARGETS = nginx_hash_test
TARGETS_C_FILE = $(TARGETS).c

CLEANUP = rm -f $(TARGETS) *.o

all: $(TARGETS)

clean:
	$(CLEANUP)

CORE_INCS = -I. \
    -I$(NGX_ROOT)/src/core \
    -I$(NGX_ROOT)/src/event \
    -I$(NGX_ROOT)/src/event/modules \
    -I$(NGX_ROOT)/src/os/unix \
    -I$(NGX_ROOT)/objs \
    -I/home/hgc/workspace/mine/c_third_party/archives/tmp/nginx_openssl/openssl-1.0.2j/include

NGX_PALLOC = $(NGX_ROOT)/objs/src/core/ngx_palloc.o
NGX_STRING = $(NGX_ROOT)/objs/src/core/ngx_string.o
NGX_ALLOC = $(NGX_ROOT)/objs/src/os/unix/ngx_alloc.o
NGX_ARRAY = $(NGX_ROOT)/objs/src/core/ngx_array.o
NGX_HASH = $(NGX_ROOT)/objs/src/core/ngx_hash.o

$(TARGETS): $(TARGETS_C_FILE)
	$(CXX) $(CXXFLAGS) $(CORE_INCS) $(NGX_PALLOC) $(NGX_STRING) $(NGX_ALLOC) $(NGX_ARRAY) $(NGX_HASH) $^ -o $@