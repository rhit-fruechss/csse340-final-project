LIBS = "src/shared.c"

pod-test:
	gcc ${LIBS} src/pod.c -o ./pod
	./pod

remote-test:
	gcc ${LIBS} src/remote.c -o ./remote
	./remote

pod-qemu:
	if [ ! -d buildroot ]; then git clone https://github.com/buildroot/buildroot.git; fi
	@echo "Copying configuration..."
	cp -f buildroot-config buildroot/.config
	@echo "Copying bootloader..."
	cp -rf bootloader buildroot/packages/custom
	@echo "Running buildroot..."
	cd buildroot
	make
	cp -f output/images/* ../output-images/*
	cd ..
