KANNEL=../gateway

all:
	@echo Start with a clean checkout of Kannel.
	@echo Edit this makefile and point KANNEL to your gateway
	@echo setup. Current value: \"${KANNEL}\".
	@echo Then type: make patch
	@echo

patch:
	patch -p0 -d ${KANNEL} < sqlbox.patch
	@for f in gw/*.c gw/*.h gwlib/*.c; do echo Copying $$f; cp -f $$f ${KANNEL}/$$f; done
	@echo If you are upgrading, read the UPGRADE document.
