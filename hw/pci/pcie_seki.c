/*
 * pcie_seki.c
 *
 * Copyright (c) 2014 Afa.L Cheng <afa@afa.moe>
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

#include "hw/pci/msi.h"

#include "pcie_seki.h"

static int pcie_seki_init(PCIDevice *dev)
{
    PCIESekiDeviceState *seki = PCIE_SEKI_DEV(dev);
    PCIEPort *p = PCIE_PORT(dev);
    int rv;

    pcie_port_init_reg(dev);

    pcie_seki_setup_mmio(dev, seki);

    rv = msi_init(dev, 0x70, 0x01,
                  PCI_MSI_FLAGS_64BIT & PCI_MSI_FLAGS_64BIT,
                  PCI_MSI_FLAGS_64BIT & PCI_MSI_FLAGS_MASKBIT);
    if (rv < 0)
        goto err;

    // 103C = Hewlett-Packard, 1172 = Altera
    rv = pci_bridge_ssvid_init(dev, 0x80, 0x1172, 0x103C);
    if (rv < 0)
        goto err;

    rv = pcie_cap_init(dev, 0x90, PCI_EXP_TYPE_ENDPOINT, p->port);
    if (rv < 0)
        goto err_msi;

    pcie_cap_flr_init(dev);
    pcie_cap_deverr_init(dev);

    // From PCIe 1.0 Spec:
    // Extended Capabilities in device configuration space always begin at
    // offset 100h with a PCI Express Enhanced Capability Header (Section 5.9.3).

    // The PCI Express Advanced Error Reporting capability is an optional
    // extended capability ...
    rv = pcie_aer_init(dev, 0x100);
    if (rv < 0)
        goto err_cap;

    return 0;

    // Errors:
err_cap:
    pcie_cap_exit(dev);
err_msi:
    msi_uninit(dev);
err:
    return rv;
}

static void pcie_seki_uninit(PCIDevice *dev)
{
    pcie_aer_exit(dev);
    pcie_cap_exit(dev);
    msi_uninit(dev);

    return;
}

static void pcie_seki_reset(DeviceState *qdev)
{
    PCIDevice *dev = PCI_DEVICE(qdev);

    pcie_cap_deverr_reset(dev);
}

static void pcie_seki_write_config(PCIDevice *dev,
                                   uint32_t address, uint32_t val, int len)
{
//    fprintf(stderr, "CW: Addr: %x, Val: %x, Len: %x\n", address, val, len);
    pci_default_write_config(dev, address, val, len);
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
    pdc->init       = pcie_seki_init;
    pdc->exit       = pcie_seki_uninit;
    pdc->config_write = pcie_seki_write_config;
    pdc->vendor_id  = 0xFA58;
    pdc->device_id  = 0x0961;
    pdc->revision   = 0x00;
    pdc->class_id   = PCI_CLASS_PROCESSOR_CO;

    dc->desc        = "Seki HPL Accelerator";
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

