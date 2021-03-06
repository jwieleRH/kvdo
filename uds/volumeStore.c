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
 * $Id: //eng/uds-releases/jasper/src/uds/volumeStore.c#2 $
 */

#include "geometry.h"
#include "indexLayout.h"
#include "logger.h"
#include "uds-error.h"
#include "volumeStore.h"


/*****************************************************************************/
void closeVolumeStore(struct volume_store *volumeStore)
{
#ifdef __KERNEL__
  if (volumeStore->vs_client != NULL) {
    dm_bufio_client_destroy(volumeStore->vs_client);
    volumeStore->vs_client = NULL;
  }
#else
  if (volumeStore->vs_region != NULL) {
    putIORegion(volumeStore->vs_region);
    volumeStore->vs_region = NULL;
  }
#endif
}

/*****************************************************************************/
void destroyVolumePage(struct volume_page *volumePage)
{
#ifdef __KERNEL__
  releaseVolumePage(volumePage);
#else
  FREE(volumePage->vp_data);
  volumePage->vp_data = NULL;
#endif
}

/*****************************************************************************/
int initializeVolumePage(const struct geometry *geometry,
                         struct volume_page    *volumePage)
{
#ifdef __KERNEL__
  volumePage->vp_buffer = NULL;
  return UDS_SUCCESS;
#else
  return ALLOCATE_IO_ALIGNED(geometry->bytesPerPage, byte, __func__,
                             &volumePage->vp_data);
#endif
}

/*****************************************************************************/
int openVolumeStore(struct volume_store *volumeStore,
                    IndexLayout  *layout,
                    unsigned int  reservedBuffers __attribute__((unused)),
                    size_t        bytesPerPage)
{
#ifdef __KERNEL__
  return openVolumeBufio(layout, bytesPerPage, reservedBuffers,
                         &volumeStore->vs_client);
#else
  volumeStore->vs_bytesPerPage = bytesPerPage;
  return openVolumeRegion(layout, &volumeStore->vs_region);
#endif
}

/*****************************************************************************/
void prefetchVolumePages(const struct volume_store *vs __attribute__((unused)),
                         unsigned int physicalPage __attribute__((unused)),
                         unsigned int pageCount __attribute__((unused)))
{
#ifdef __KERNEL__
  dm_bufio_prefetch(vs->vs_client, physicalPage, pageCount);
#else
  // Nothing to do in user mode
#endif
}

/*****************************************************************************/
int prepareToWriteVolumePage(const struct volume_store *volumeStore
                             __attribute__((unused)),
                             unsigned int         physicalPage
                             __attribute__((unused)),
                             struct volume_page  *volumePage
                             __attribute__((unused)))
{
#ifdef __KERNEL__
  releaseVolumePage(volumePage);
  struct dm_buffer *buffer = NULL;
  byte *data = dm_bufio_new(volumeStore->vs_client, physicalPage, &buffer);
  if (IS_ERR(data)) {
    return -PTR_ERR(data);
  }
  volumePage->vp_buffer = buffer;
#else
  // Nothing to do in user mode
#endif
  return UDS_SUCCESS;
}

/*****************************************************************************/
int readVolumePage(const struct volume_store *volumeStore,
                   unsigned int               physicalPage,
                   struct volume_page        *volumePage)
{
#ifdef __KERNEL__
  releaseVolumePage(volumePage);
  byte *data = dm_bufio_read(volumeStore->vs_client, physicalPage,
                             &volumePage->vp_buffer);
  if (IS_ERR(data)) {
    return logWarningWithStringError(-PTR_ERR(data),
                                     "error reading physical page %u",
                                     physicalPage);
  }
#else
  off_t offset = (off_t) physicalPage * volumeStore->vs_bytesPerPage;
  int result = readFromRegion(volumeStore->vs_region, offset,
                              getPageData(volumePage),
                              volumeStore->vs_bytesPerPage, NULL);
  if (result != UDS_SUCCESS) {
    return logWarningWithStringError(result,
                                     "error reading physical page %u",
                                     physicalPage);
  }
#endif
  return UDS_SUCCESS;
}

/*****************************************************************************/
void releaseVolumePage(struct volume_page *volumePage __attribute__((unused)))
{
#ifdef __KERNEL__
  if (volumePage->vp_buffer != NULL) {
    dm_bufio_release(volumePage->vp_buffer);
    volumePage->vp_buffer = NULL;
  }
#else
  // Nothing to do in user mode
#endif
}

/*****************************************************************************/
void swapVolumePages(struct volume_page *volumePage1,
                     struct volume_page *volumePage2)
{
  struct volume_page temp = *volumePage1;
  *volumePage1 = *volumePage2;
  *volumePage2 = temp;
}

/*****************************************************************************/
int syncVolumeStore(const struct volume_store *volumeStore)
{
#ifdef __KERNEL__
  int result = -dm_bufio_write_dirty_buffers(volumeStore->vs_client);
#else
  int result = syncRegionContents(volumeStore->vs_region);
#endif
  if (result != UDS_SUCCESS) {
    return logErrorWithStringError(result, "cannot sync chapter to volume");
  }
  return UDS_SUCCESS;
}

/*****************************************************************************/
int writeVolumePage(const struct volume_store *volumeStore,
                    unsigned int               physicalPage,
                    struct volume_page        *volumePage)
{
#ifdef __KERNEL__
  dm_bufio_mark_buffer_dirty(volumePage->vp_buffer);
  return UDS_SUCCESS;
#else
  off_t offset = (off_t) physicalPage * volumeStore->vs_bytesPerPage;
  return writeToRegion(volumeStore->vs_region, offset, getPageData(volumePage),
                       volumeStore->vs_bytesPerPage,
                       volumeStore->vs_bytesPerPage);
#endif
}
