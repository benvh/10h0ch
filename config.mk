target:=bin/10h0ch

src_dir := src
obj_dir := .o
dep_dir := .d

srcs := $(shell find $(src_dir) -name "*.c" -type f)
hdrs := $(shell find $(src_dir) -name "*.h" -type f)
objs := $(patsubst %,$(obj_dir)/%.o,$(basename $(srcs)))
deps := $(patsubst .o/%,.d/%,$(objs:.o=.d))

CC = clang
PKGCONFIG := pkg-config

CFLAGS = -Wall -g -std=c99 -pedantic \
		 -I $(src_dir) \
		 $(shell $(PKGCONFIG) --cflags sdl2 SDL2_image SDL2_ttf) \
		 $(shell $(PKGCONFIG) --cflags fontconfig)

LDFLAGS = $(shell $(PKGCONFIG) --libs sdl2 SDL2_image SDL2_ttf) \
		  $(shell $(PKGCONFIG) --libs fontconfig)
