/*
 * Copyright (c) 2020 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA. 
 *
 * $Id: //eng/vdo-releases/aluminum/src/c++/vdo/kernel/statusCodeBlocks.h#1 $
 */

#ifndef STATUS_CODE_BLOCKS_H
#define STATUS_CODE_BLOCKS_H

enum {
  UDS_BLOCK_SIZE  = UDS_ERROR_CODE_BLOCK_END - UDS_ERROR_CODE_BASE,
  VDO_BLOCK_START = UDS_ERROR_CODE_BLOCK_END,
  VDO_BLOCK_END   = VDO_BLOCK_START + UDS_BLOCK_SIZE,
  PRP_BLOCK_START = VDO_BLOCK_END,
  PRP_BLOCK_END   = PRP_BLOCK_START + UDS_BLOCK_SIZE,
};

#endif // STATUS_CODE_BLOCKS_H
