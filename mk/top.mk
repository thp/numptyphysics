include mk/vars.mk
include mk/funcs.mk
include mk/silent.mk
include mk/rules.mk

$(eval $(call include_makefile,mk/main.mk))

include mk/pkgs.mk
include mk/deps.mk
include mk/objs.mk
include mk/install.mk
