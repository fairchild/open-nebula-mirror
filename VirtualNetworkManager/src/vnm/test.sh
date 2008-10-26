cd ../../
g++ -o src/vnm/fl_test.o -c -g -Wall -Iinclude -I/usr/include src/vnm/fl_test.cc

g++ -o src/vnm/fl_test src/vnm/fl_test.o -Lsrc/common -Lsrc/host -Lsrc/mad -Lsrc/nebula -Lsrc/pool -Lsrc/template -Lsrc/vm -Lsrc/vmm -Lsrc/lcm -Lsrc/tm -Lsrc/dm -Lsrc/im -Lsrc/rm -Lsrc/vnm -L/usr/lib -lxmlrpc_client -lxmlrpc_client++ -lnebula_core -lnebula_vmm -lnebula_lcm -lnebula_vm -lnebula_im -lnebula_rm -lnebula_dm -lnebula_tm -lnebula_mad -lnebula_template -lnebula_pool -lnebula_host -lnebula_common -lsqlite3 -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_server_abyss -lxmlrpc_server -lxmlrpc_abyss -lpthread -lxmlrpc++ -lxmlrpc -lxmlrpc_util -lxmlrpc_xmlparse -lxmlrpc_xmltok -lnebula_vnm

cd -
