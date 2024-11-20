#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mdadm.h"
#include "jbod.h"

int is_mounted = 0;

// Mount the drive
int mdadm_mount(void) {
    if (is_mounted) {
        return -1;
    }

	//JBOD_MOUNT << 12 is a bitwsie left shift operator effectively multiplies the number by 2^12
	//This declares a 32-bit unsigned integer op is short for "operation code
    uint32_t op = JBOD_MOUNT << 12; 
    if (!jbod_operation(op, NULL)) {
        is_mounted = 1;
        return 1;
    } else {
        return -1;
    }
}

// Unmount the drive
int mdadm_unmount(void) {
    if (!is_mounted) {
        return -1;
    }
    uint32_t op = JBOD_UNMOUNT << 12;
    if (!jbod_operation(op, NULL)) {
        is_mounted = 0;
        return 1;
    } else {
        return -1;
    }
}

// Read from the disk
int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf) {
    int remaining_bytes = read_len;

    // Check if the read would be out of bounds
    if (start_addr + read_len > 65536 * 16) {
        return -1;
    }

    // Check if the read length exceeds the maximum allowed size
    if (read_len > 1024) {
        return -2;
    }

    // Ensure the drive is mounted
    if (!is_mounted) {
        return -3;
    }

    // Ensure the buffer is valid
    if ((read_buf == NULL) && (read_len > 0)) {
        return -4;
    }
	
	//a 8-bit unsigned integer is a byte
    uint8_t temp_block[256];// makes a temp array of size 256 bytes
    uint8_t *block_pointer = temp_block; // points to a byte
	//Now, block_pointer can be used to access or manipulate the bytes of the temp_block array in byte-by-byte operations.

    // Calculate disk and block IDs from the starting address
    int disk_index = start_addr / JBOD_DISK_SIZE;//index of the disk where the starting address falls.
	int base_address_of_calculated_disk = disk_index * JBOD_DISK_SIZE;
	int offset_by_the_start_addr = start_addr - base_address_of_calculated_disk;
    int block_index_in_disk = (offset_by_the_start_addr) / JBOD_BLOCK_SIZE;

    // Seek to the appropriate disk and block
    jbod_operation(JBOD_SEEK_TO_DISK << 12 | disk_index | block_index_in_disk << 4, NULL);//first seek to correct disk
    jbod_operation(JBOD_SEEK_TO_BLOCK << 12 | disk_index | block_index_in_disk << 4, NULL);//then seek to correct block in disk
	//As data is read from the disk, it will be written into the memory location pointed to by destination_memory_buffer_pointer
    uint8_t *destination_memory_buffer_pointer = read_buf;//byte type ,working pointer that can be changed without modifying the original read_buf pointer. 
    int block_offset = start_addr % 256;  // Offset for the first block
    int is_first_block = 1;  // Tracks if the first block offset is handled
    int disk_switched = 0;  // Tracks whether disks were switched
    int current_addr = start_addr;  // Current address being read

    // Read the initial block
    jbod_operation(disk_index | block_index_in_disk << 4 | JBOD_READ_BLOCK << 12, temp_block);
    block_index_in_disk++;

    while (remaining_bytes > 0) {
        // Handle switching disks if needed
        if ((current_addr % JBOD_DISK_SIZE == 0) && (current_addr > 0) && !disk_switched) {
            disk_index = current_addr / JBOD_DISK_SIZE;
            block_index_in_disk = (current_addr - disk_index * JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;

            jbod_operation(JBOD_SEEK_TO_DISK << 12 | disk_index | block_index_in_disk << 4, NULL);
            jbod_operation(JBOD_SEEK_TO_BLOCK << 12 | disk_index | block_index_in_disk << 4, NULL);

            jbod_operation(disk_index | block_index_in_disk << 4 | JBOD_READ_BLOCK << 12, temp_block);
            block_index_in_disk++;
            block_pointer = temp_block;
            disk_switched = 1;
        } 
        // Handle reading for the first block if offset exists and exceeds the block size
        else if (block_offset + remaining_bytes > JBOD_BLOCK_SIZE && is_first_block) {
            block_pointer += block_offset;  // Use pointer arithmetic instead of increment_ptr
            memcpy(destination_memory_buffer_pointer, block_pointer, 256 - block_offset);

            destination_memory_buffer_pointer += 256 - block_offset;
            current_addr += 256 - block_offset;
            remaining_bytes -= 256 - block_offset;

            block_offset = 0;
            is_first_block = 0;

            // Read the next block
            jbod_operation(disk_index | block_index_in_disk << 4 | JBOD_READ_BLOCK << 12, temp_block);
            block_index_in_disk++;
            block_pointer = temp_block;
        } 
        // Handle reading a full block
        else if (!is_first_block && remaining_bytes > 256) {
            memcpy(destination_memory_buffer_pointer, block_pointer, 256);

            destination_memory_buffer_pointer += 256;
            current_addr += 256;
            remaining_bytes -= 256;

            // Read the next block
            jbod_operation(disk_index | block_index_in_disk << 4 | JBOD_READ_BLOCK << 12, temp_block);
            block_index_in_disk++;
            block_pointer = temp_block;
        } 
        // Handle reading the last block or a partial block
        else {
            block_pointer += block_offset;  // Use pointer arithmetic instead of increment_ptr
            memcpy(destination_memory_buffer_pointer, block_pointer, remaining_bytes);

            remaining_bytes = 0;
        }
    }

    return read_len;
}

