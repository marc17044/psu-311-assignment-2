[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/KO-MdZgM)
# Assignment-2

## **CMPSC 311 Fall 2024 Lab 2 – mdadm Linear Device**

### **General Instructions:**

For this lab assignment, you **must not** copy any content from the Internet or discuss your work with anyone. This includes ideas, code, configuration, or text. You are also prohibited from receiving help from anyone inside or outside the class.

There is no library function that is necessarily needed to accomplish this assignment, but you may use if you want. 

**Do not** use any library functions that is **not** in the included header files (e.g., stdlib.h, string.h) provided, using those results in undefined symbol and compilation error - you will get 0. 

**Do not** try to include any extra header files to make your implementation easier, we will detect them. 

You **do not** need to (re)implement main function in this assignment, as it is provided to you in tester.c already. Your task in this project is to implement the **3 required functions** (details will be introduced below).

### **Compilation:**

Use **gcc-9** to compile your code. Follow these steps to switch to gcc-9 on the lab machines:

1. Run the command:  
   `/home/software/user\_conf/bin/new\_soft  `
   This will create a file named \~/.software.  
2. Open the file using:  
   `vi \~/.software  `
3. In the file, find the lines related to the gcc version and uncomment the one for gcc-9.5.0. It should look like this: `gcc-9.5.0  `

Note: Ensure that only gcc-9.5.0 is uncommented, and all other lines remain commented. 
```
# gcc-11.3.0: GCC 11.3.0 compiler
#gcc-11.3.0

# gcc-9.5.0: GCC 9.5.0 compiler
gcc-9.5.0
```

4. Log out of the machine by closing VSCode.  
5. Reopen VSCode and reconnect to the lab machine.  
6. Verify the gcc version by running:  
   `which gcc `
   You should see that it’s set to  `gcc-9`.

### 

### 

### **Backstory of this Assignment:**

Today is your first day at a cryptocurrency startup for your summer internship. The marketing team has decided to emphasize security as a selling point.

Coincidentally, a shipment of **16 military-grade, nuclear bomb-proof hard disks** has just arrived. These disks will store the most critical user data—cryptocurrency wallets. However, the company that makes the disks focuses more on physical security than software.

They provided you with the disks in a **JBOD (Just a Bunch of Disks)** setup. JBOD is a storage architecture consisting of several disks inside a single enclosure. Along with the disks, they’ve provided you with a user manual that describes how to interact with them.

### **Disk Operation Format:**

| Bits | Width | Field | Description |
| :---: | :---: | :---: | :---: |
| 0-3 | 4 | DiskID | ID of the disk for the operation |
| 4-11 | 8 | BlockID | ID of the block within the disk |
| 12-19 | 8 | Command | The command to be executed |
| 20-31 | 12 | Reserved | Unused bits |

Each of these hard disks consists of `i` blocks, with each block holding `256` bytes. Since you’ve bought `j` disks, the total storage capacity is:  
`i × j × 256 = 1,048,576 bytes = 1 MB`  
Both `i` and `j` are **unknown** integers.

The following function can be used to control the disks:  
`int jbod_operation(uint32_t op, uint8_t *block);`

This function returns `0` on success and `-1` on failure. It takes an operation (`op`) in the format described in the above table and a pointer to a buffer. The `op` field’s value can be one of the following (defined as a C `enum` in the provided header):

1. **JBOD\_MOUNT**: Mounts all disks. This must be the first command to be issued; otherwise, all commands will fail. When the command field of `op` is set to this command, all other fields in `op` are ignored by the JBOD driver, i.e., the block argument passed to `jbod_operation` can be NULL.  
2. **JBOD\_UNMOUNT**: Unmounts all disks. This should be the last command to be issued. Similar to JBOD\_MOUNT, the other fields and the `block` argument are ignoredi.e., the block argument passed to `jbod_operation` can be NULL.  
3. **JBOD\_SEEK\_TO\_DISK**: Moves the I/O position to a specific disk. The JBOD maintains an I/O position as a tuple `{CurrentDiskID, CurrentBlockID}`, which determines where the next I/O operation will happen. This command seeks to the beginning of disk specified by `DiskID` field in `op`. In other words, it modifies I/O position: it sets `CurrentDiskID` to `DiskID` specified in `op` and it sets `CurrentBlockID` to 0\. When the command field of `op` is set to this command, the `BlockID` field in `op` is ignored by the JBOD driver.  
4. **JBOD\_SEEK\_TO\_BLOCK**: Moves the I/O position to a specific block in the current disk. This command modifies the `CurrentBlockID` to the block specified in `BlockID` field in `op`, When the command field of `op` is set to this command, the `DiskID` field in `op` is ignored by the JBOD driver.  
5. **JBOD\_READ\_BLOCK**: Reads the block from the current I/O position into the buffer specified by the `block` argument of `jbod_operation`. The buffer must be 256 bytes (block size). After this operation, the `CurrentBlockID` is incremented by 1, so the next read occurs at the next block. When the command field of `op` is set to this command, all other fields in op are ignored by the JBOD driver.  
   

### **Your Task:**

After your onboarding session and lunch, you receive an email from your manager outlining your assignment for the next two weeks. You’ll be integrating JBOD into the company’s existing storage system. Specifically, you’ll implement a feature from the **mdadm** utility in Linux, which manages multiple disks. Your job is to implement the **linear device** functionality, which makes multiple disks appear as one large disk.

You will work with `j` disks, each of size `k` `KB`, to create a 1 MB disk. Before implementing the functions, fill out the appropriate values for `JBOD_NUM_DISKS`, `JBOD_DISK_SIZE`, `JBOD_BLOCK_SIZE`, and `JBOD_NUM_BLOCKS_PER_DISK` in the `jbod.h` file.

**Note:** those values are related to testing your submission, if you input inappropriate values that result in the testing script failing to run, you will get an 0\. Coming up with those values is the first step of this assignment before you implement the following functions.

**Functions to Implement:**

1. **mdadm\_mount**:  
   Mounts the linear device. After this, users can read from and write to the combined linear address space. It should return `1` on success and `-1` on failure. More than a single call mdadm\_mount should fail.  
2. **mdadm\_unmount**:  
   Unmounts the linear device. All further operations should fail. It should return `1` on success and `-1` on failure. More than a single call to mdadm\_unmount should fail.  
3. **mdadm\_read**:  
   Reads `read_len` bytes from the combined address space starting at `start_addr` into `read_buf`. The function returns the number of bytes it reads when it succeeds.   
* Reading beyond the valid address space should fail. The function should return \-1 in this case.   
* `read_len` cannot exceed 1024 bytes. In this case it should return \-2.   
* Reading when the system is unmounted should fail. In this case it should return \-3.   
* There may be additional restrictions that should cause mdadm\_read to fail. You’ll encounter these cases as you pass the tests. Return \-4 for them. 

Now you are all pumped up and ready to make an impact in the new company. You spend the afternoon with your mentor, who goes through the directory structure and the development procedure with you:

1\. jbod.h: The interface of JBOD. You will use the constants defined here in your implementation. ( NOTE : you have to edit the right values in this file)  
2\. jbod.o: The object file containing the JBOD driver.  
3\. mdadm.h: A header file that lists the functions you should implement.  
4\. mdadm.c: Your implementation of mdadm functions. ( NOTE : You will edit this file with all your implementations)  
5\. tester.h: Tester header file.  
6\. tester.c: Unit tests for the functions that you will implement. This file will compile into an executa-ble, tester, which you will run to see if you pass the unit tests.  
7\. util.h: Utility functions used by JBOD implementation and the tester.  
8\. util.c: Implementation of utility functions.  
9\. Makefile: instructions for compiling and building tester used by the make utility.

### 

### **Expected Workflow:**

Your workflow consists of:

1. Editing `jbod.h` with the correct values.  
2. Implementing the functions in `mdadm.c`.  
3. Running `make clean` to remove object files.  
4. Running `make` to build the tester.  
5. Running `./tester` to verify if your implementation passes the tests.

Repeat these steps until you pass all the tests. Although you only need to modify `jbod.h` and `mdadm.c`, you are allowed to modify other files if it helps you. However, when we test your submission, we will use the original versions of all files except `jbod.h`, `mdadm.c`, and `mdadm.h`.

Feel free to create helper functions if needed. For example, a helper function could determine which block and disk correspond to a specific linear address.

### **Honors Option:**

**1\. Answer the Following Question**

The JBOD system supports several commands, such as `JBOD_SEEK_TO_DISK`, `JBOD_SEEK_TO_BLOCK`, and `JBOD_READ_BLOCK`. Imagine you are tasked with designing a new JBOD operation **`JBOD_SEQUENTIAL_READ`** to improve efficiency for performing **large sequential read operations**. However, you must respect the constraint that each JBOD block is 256 bytes and that each read operation can only process a single block at a time.

Given these constraints, propose a new JBOD command that could help optimize sequential reads across large blocks of data while maintaining the existing command structure. Explain how this new command (1) would be implemented, (2) its advantages, and (3) why it would be useful for JBOD systems in real-world applications.

**2\. Coding**

Modify the `mdadm_read` function to handle cases where the read request starts within a valid block but attempts to read past the end of the **disk**. Instead of reading beyond the disk, the function should return an error if the read would cross the disk boundary. Make sure that no more than 1024 bytes are read in total, and the read should stop at the end of the disk if it reaches the boundary.

1. Modify the `mdadm_read` function to implement this behavior.  
2. Write a test case to validate that the function correctly handles reads that attempt to cross the disk boundary.

### **Grading:**

* **Make errors** will result in a score of 0\. Ensure your code compiles without errors.  
* The assignment includes **10 test cases**, each worth 1 point.  
* **Comments** in your code are recommended for clarity.

### **Penalties:**

* **Late submissions** will incur a 10% penalty per day, up to 3 days.  
* After 3 days, the lab will no longer be graded.

