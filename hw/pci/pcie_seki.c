/*
 * pcie_seki.c
 *
 * Copyright (c) 2014 Afa.L Cheng <alpha@alpha.moe>
 *                    Rosen Center for Advanced Computing, Purdue University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hw/pci/pcie_port.h"
#include "hw/pci/msi.h"
#include "hw/pci/pci_bridge.h"

typedef struct PCIE_Seki_Device_State {
    PCIEPort parent_obj;
} PCIESekiDeviceState;

#define TYPE_PCIE_SEKI_DEVICE "pcie-seki"

#define PCIE_SEKI_DEV(obj) \
    OBJECT_CHECK(PCIESekiDeviceState, (obj), TYPE_PCIE_SEKI_DEVICE)

static int pcie_seki_init(PCIDevice *dev)
{
    PCIESekiDeviceState *seki = PCIE_SEKI_DEV(dev);
    PCIEPort *p = PCIE_PORT(dev);
    int rv;

    (void)(seki);

    // Init PCIe Port Reg
    pcie_port_init_reg(dev);

    rv = msi_init(dev, 0x70, 0x01,
                  PCI_MSI_FLAGS_64BIT & PCI_MSI_FLAGS_64BIT,
                  PCI_MSI_FLAGS_64BIT & PCI_MSI_FLAGS_MASKBIT);
    if (rv < 0) {
        fprintf(stderr, "MSI init failed: %d\n", rv);
        abort();
    }

    rv = pci_bridge_ssvid_init(dev, 0x80, 0, 0);
    if (rv < 0) {
        fprintf(stderr, "SSVID init failed: %d\n", rv);
        abort();
    }

    rv = pcie_cap_init(dev, 0x90, PCI_EXP_TYPE_ENDPOINT, p->port);
    if (rv < 0) {
        fprintf(stderr, "PCIE CAP init failed: %d\n", rv);
        abort();
    }

    pcie_cap_flr_init(dev);
    pcie_cap_deverr_init(dev);

    rv = pcie_aer_init(dev, 0x100);
    if (rv < 0) {
        fprintf(stderr, "PCIE AER init failed: %d\n", rv);
        abort();
    }

    return 0;
}

static void pcie_seki_uninit(PCIDevice *dev)
{
    pcie_aer_exit(dev);
    pcie_cap_exit(dev);
    msi_uninit(dev);
    pci_bridge_exitfn(dev);

    return;
}

static void pcie_seki_reset(DeviceState *qdev)
{
    PCIDevice *dev = PCI_DEVICE(qdev);

    pcie_cap_deverr_reset(dev);
    pci_bridge_reset(qdev);
}

static void pcie_seki_write_config(PCIDevice *dev,
                                   uint32_t address, uint32_t val, int len)
{
//    pci_bridge_write_config(dev, address, val, len);
    pcie_cap_flr_write_config(dev, address, val, len);
    pcie_aer_write_config(dev, address, val, len);
}

static const VMStateDescription vmstate_pcie_seki = {
    .name = "pcie-seki",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_PCIE_DEVICE(parent_obj.parent_obj, PCIEPort),
        VMSTATE_STRUCT(parent_obj.parent_obj.exp.aer_log, PCIEPort, 0,
                       vmstate_pcie_aer_log, PCIEAERLog),
        VMSTATE_END_OF_LIST()
    }
};

static void pcie_seki_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    PCIDeviceClass *pdc = PCI_DEVICE_CLASS(oc);

    pdc->is_express = 1;
//    pdc->is_bridge  = 1;
    pdc->init       = pcie_seki_init;
    pdc->exit       = pcie_seki_uninit;
    pdc->config_write = pcie_seki_write_config;
    pdc->vendor_id  = 0xFA58;
    pdc->device_id  = 0x0961;
    pdc->revision   = 0x00;
    pdc->class_id   = PCI_CLASS_PROCESSOR_CO;

    dc->desc        = "Seki Device";
    dc->reset       = pcie_seki_reset;
    dc->vmsd        = &vmstate_pcie_seki;
}

static const TypeInfo pcie_seki_info = {
    .name           = TYPE_PCIE_SEKI_DEVICE,
    .parent         = TYPE_PCIE_PORT,
    .instance_size  = sizeof(PCIESekiDeviceState),
    .class_init     = pcie_seki_class_init,
};


static void pcie_seki_register_types(void)
{
    type_register_static(&pcie_seki_info);
}

type_init(pcie_seki_register_types)

