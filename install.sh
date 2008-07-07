#!/bin/sh

SRC_DIR=$PWD
DST_DIR=$1

echo $SRC_DIR
echo $DST_DIR

inst_ln() {
    ln -s $SRC_DIR/$1 $DST_DIR/$2
}

inst_cp() {
    cp $SRC_DIR/$1 $DST_DIR/$2
}

if [ -z "$SRC_DIR" -o -z "$DST_DIR" ]; then
    echo Must supply a destination directory
    exit -1
fi

DIRS="/bin /etc /etc/im_kvm /etc/im_xen /etc/vmm_kvm /etc/vmm_xen /libexec /lib/ruby /var /share/examples /lib/im_probes"

for d in $DIRS; do
    mkdir -p $DST_DIR$d
done

inst_ln src/nebula/oned bin
inst_ln src/scheduler/mm_sched bin
inst_ln src/client/ruby/onevm bin
inst_ln src/client/ruby/onehost bin

inst_cp share/etc/oned.conf etc
inst_ln share/etc/mad/defaultrc etc

inst_ln share/scripts/madcommon.sh libexec

inst_ln share/etc/mad/vmm_xenrc etc/vmm_xen
inst_ln share/etc/default/vmm_xen.conf etc/vmm_xen
inst_ln share/etc/mad/vmm_kvmrc etc/vmm_kvm
inst_ln share/etc/default/vmm_kvm.conf etc/vmm_kvm

inst_ln src/vmm_mad/xen/one_vmm_xen.rb bin
inst_ln src/vmm_mad/xen/one_vmm_xen bin

inst_ln src/vmm_mad/kvm_ssh/one_vmm_kvm.rb bin
inst_ln src/vmm_mad/kvm_ssh/one_vmm_kvm bin

inst_ln src/im_mad/im_ssh/one_im_ssh.rb bin
inst_ln src/im_mad/im_ssh/one_im_ssh bin
inst_ln src/im_mad/im_ssh/one_ssh.rb lib/ruby

inst_ln share/etc/mad/im_kvmrc etc/im_kvm
inst_cp src/im_mad/kvm/im_kvm.conf etc/im_kvm
inst_cp src/im_mad/xen/im_xen.conf etc/im_xen
inst_ln share/etc/mad/im_xenrc etc/im_xen


inst_ln src/vmm_mad/xen/one_mad.rb lib/ruby
inst_ln src/client/ruby/one.rb lib/ruby
inst_ln src/client/ruby/client_utilities.rb lib/ruby
inst_ln src/client/ruby/command_parse.rb lib/ruby

inst_ln src/im_mad/im_ssh/architecture.sh lib/im_probes
inst_ln src/im_mad/im_ssh/cpu.sh lib/im_probes
inst_ln src/im_mad/im_ssh/name.sh lib/im_probes
inst_ln src/im_mad/xen/xen.rb lib/im_probes
inst_ln src/im_mad/kvm/kvm.rb lib/im_probes

inst_cp share/scripts/one bin

inst_cp share/examples/vm.template share/examples


