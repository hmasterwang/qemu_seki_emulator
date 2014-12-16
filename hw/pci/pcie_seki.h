/*
 * pcie_seki.h
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

#ifndef PCIE_SEKI_H
#define PCIE_SEKI_H

#include "hw/pci/pcie_port.h"


#define TYPE_PCIE_SEKI_DEVICE "pcie-seki"


typedef struct PCIE_Seki_Device_State {
    PCIEPort parent_obj;

    MemoryRegion ctrl_memregion;
    MemoryRegion input_memregion;
    MemoryRegion output_memregion;

} PCIESekiDeviceState;


#define PCIE_SEKI_DEV(obj) \
    OBJECT_CHECK(PCIESekiDeviceState, (obj), TYPE_PCIE_SEKI_DEVICE)


// Structs
extern const MemoryRegionOps seki_ctrl_memregion_ops;
extern const MemoryRegionOps seki_input_memregion_ops;
extern const MemoryRegionOps seki_output_memregion_ops;


// Functions
void pcie_seki_setup_mmio(PCIDevice *dev, PCIESekiDeviceState *seki);


#endif // PCIE_SEKI_H
