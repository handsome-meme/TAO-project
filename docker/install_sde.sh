!!; while [ $? -ne 0 ]; do !!; done

逐个安装依赖

./install_libcli.py --os-name Debian --os-version 9 --jobs 8 --sde-install /home/buildsde/bf-sde-9.7.0/install --keyword apt-get --with-proto yes
./install_grpc.py --os-name Debian --os-version 9 --jobs 8 --sde-install /home/buildsde/bf-sde-9.7.0/install --keyword apt-get --with-proto yes



libbpf-dev libbsd-dev libibverbs-dev  libipsec-mb-dev libisal-dev  libfdt-dev libjansson-dev 