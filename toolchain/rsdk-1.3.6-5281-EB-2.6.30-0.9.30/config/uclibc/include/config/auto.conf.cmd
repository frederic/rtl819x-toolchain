deps_config := \
	extra/Configs/Config.in.arch \
	extra/Configs/Config.rlx \
	./extra/Configs/Config.in

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
