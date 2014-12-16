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

#include "hw/pci/pcie_port.h"
#include "hw/pci/msi.h"

typedef struct PCIE_Seki_Device_State {
    PCIEPort parent_obj;

    MemoryRegion ctrl_memregion;
    MemoryRegion input_memregion;
    MemoryRegion output_memregion;

} PCIESekiDeviceState;

#define TYPE_PCIE_SEKI_DEVICE "pcie-seki"

#define PCIE_SEKI_DEV(obj) \
    OBJECT_CHECK(PCIESekiDeviceState, (obj), TYPE_PCIE_SEKI_DEVICE)


// Ctrl Memory Region
static uint64_t
seki_ctrl_memregion_read(void *opaque, hwaddr addr, unsigned size)
{
    fprintf(stderr, "CTRL MMIO Read at %16lx, size %8x\n", addr, size);
    return 0;
}

static void
seki_ctrl_memregion_write(void *opaque, hwaddr addr, uint64_t val,
                 unsigned size)
{
    fprintf(stderr, "CTRL MMIO Write at %16lx, size %8x, value %lx\n",
            addr, size, val);
}

static const MemoryRegionOps seki_ctrl_memregion_ops = {
    .read   = seki_ctrl_memregion_read,
    .write  = seki_ctrl_memregion_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

// Input Memory Region
static uint64_t
seki_input_memregion_read(void *opaque, hwaddr addr, unsigned size)
{
    fprintf(stderr, "INPUT MMIO Read at %16lx, size %8x\n", addr, size);
    return 0;
}

static void
seki_input_memregion_write(void *opaque, hwaddr addr, uint64_t val,
                 unsigned size)
{
    fprintf(stderr, "INPUT MMIO Write at %16lx, size %8x, value %lx\n",
            addr, size, val);
}

static const MemoryRegionOps seki_input_memregion_ops = {
    .read   = seki_input_memregion_read,
    .write  = seki_input_memregion_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

// Output Memory Region
static uint64_t
seki_output_memregion_read(void *opaque, hwaddr addr, unsigned size)
{
    fprintf(stderr, "OUTPUT MMIO Read at %16lx, size %8x\n", addr, size);
    return 0;
}

static void
seki_output_memregion_write(void *opaque, hwaddr addr, uint64_t val,
                 unsigned size)
{
    fprintf(stderr, "OUTPUT MMIO Write at %16lx, size %8x, value %lx\n",
            addr, size, val);
}

static const MemoryRegionOps seki_output_memregion_ops = {
    .read   = seki_output_memregion_read,
    .write  = seki_output_memregion_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};
// End of MMIO functions/data structures


static int pcie_seki_init(PCIDevice *dev)
{
    PCIESekiDeviceState *seki = PCIE_SEKI_DEV(dev);
    PCIEPort *p = PCIE_PORT(dev);
    int rv;

    pcie_port_init_reg(dev);

    // Setup MMIO, each 64-bit BAR takes 2 registers
    // Control/Status Registers
    memory_region_init_io(&seki->ctrl_memregion, OBJECT(seki), &seki_ctrl_memregion_ops,
                          seki, "seki-mmio", 0x100000); // 1MB 0x100000

    pci_register_bar(dev, 0, PCI_BASE_ADDRESS_MEM_TYPE_64 |
                     PCI_BASE_ADDRESS_SPACE_MEMORY, &seki->ctrl_memregion);

    // Input Memory
    memory_region_init_io(&seki->input_memregion, OBJECT(seki), &seki_input_memregion_ops,
                          seki, "seki-mmio", 0x8000000); // 128MB 0x8000000

    pci_register_bar(dev, 2, PCI_BASE_ADDRESS_MEM_TYPE_64 |
                     PCI_BASE_ADDRESS_SPACE_MEMORY, &seki->input_memregion);

    // Output Memory
    memory_region_init_io(&seki->output_memregion, OBJECT(seki), &seki_output_memregion_ops,
                          seki, "seki-mmio", 0x4000000); // 64MB 0x8000000

    pci_register_bar(dev, 4, PCI_BASE_ADDRESS_MEM_TYPE_64 |
                     PCI_BASE_ADDRESS_SPACE_MEMORY, &seki->output_memregion);
    // End of setup MMIO.
    // TODO: Extract MMIO setup to new function


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

