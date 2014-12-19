/*
 * pcie_seki_mmio.c
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

#include "pcie_seki.h"

// Ctrl Memory Region
static uint64_t
seki_ctrl_memregion_read(void *opaque, hwaddr addr, unsigned size)
{
    fprintf(stderr, "CTRL MMIO Read at 0x%16lx, size 0x%8x\n", addr, size);
    return 0;
}

static void
seki_ctrl_memregion_write(void *opaque, hwaddr addr, uint64_t val,
                 unsigned size)
{
    fprintf(stderr, "CTRL MMIO Write at 0x%16lx, size 0x%8x, value 0x%lx\n",
            addr, size, val);
}

const MemoryRegionOps seki_ctrl_memregion_ops = {
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
    fprintf(stderr, "INPUT MMIO Read at 0x%16lx, size 0x%8x\n", addr, size);
    return 0;
}

static void
seki_input_memregion_write(void *opaque, hwaddr addr, uint64_t val,
                 unsigned size)
{
    fprintf(stderr, "INPUT MMIO Write at 0x%16lx, size 0x%8x, value 0x%lx\n",
            addr, size, val);
}

const MemoryRegionOps seki_input_memregion_ops = {
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
    fprintf(stderr, "OUTPUT MMIO Read at 0x%16lx, size 0x%8x\n", addr, size);
    return 0;
}

static void
seki_output_memregion_write(void *opaque, hwaddr addr, uint64_t val,
                 unsigned size)
{
    fprintf(stderr, "OUTPUT MMIO Write at 0x%16lx, size 0x%8x, value 0x%lx\n",
            addr, size, val);
}

const MemoryRegionOps seki_output_memregion_ops = {
    .read   = seki_output_memregion_read,
    .write  = seki_output_memregion_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};
// End of MMIO functions/data structures



void pcie_seki_setup_mmio(PCIDevice *dev, PCIESekiDeviceState *seki)
{
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
}
