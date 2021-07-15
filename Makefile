include config.mk

all: $(target)

$(target): $(objs)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(dep_dir)/%.d: %.c
	@mkdir -p $(@D); \
		rm -f $@; \
		$(CC) $(CFLAGS) -E -MM -MP -MF $@__ $<; \
		sed 's,\($(basename $(<F))\)\.o[ :]*,$(obj_dir)/$(<D)/\1.o $@: ,g' $@__ > $@; \
		rm -f $@__

$(obj_dir)/%.o:
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(dep_dir)/*
	rm -rf $(obj_dir)/*
	rm -f $(target)


.PHONY: all clean

include $(deps)
