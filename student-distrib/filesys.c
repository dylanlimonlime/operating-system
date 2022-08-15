/* filesys.c
 * contains initialization and functions for reading
 * and writing files and directories
 */

#include "filesys.h"

#define SUCCESS 0
#define FAILURE -1

/* ~~~~~~~~~~~~~~~~~~~~ INITITIALIZATION ~~~~~~~~~~~~~~~~~~~~ */

/* global boot block variables */
uint32_t boot_block_addr;       //base address of boot block
boot_block_t* boot_block;       //ptr to boot block
inode_t* inode_start;           //ptr to start of inodes
data_block_t* data_block_start; //ptr to start of data_block

/* current file location */
//uint32_t curr_file_location;

/* current dentry index */
uint32_t curr_dentry_idx;
dentry_t curr_file;

/*
 * filesys_init(uint32_t multiboot_module_addr)
 * Description: Initializes the file system
 * Inputs: multiboot_module_addr - starting address of multiboot module
 * Outputs: NONE
 * Side Effects: Initialized boot block
 */
void filesys_init(uint32_t multiboot_module_addr){
    boot_block_addr = multiboot_module_addr;
    boot_block = (boot_block_t*)boot_block_addr;                                    //boot block ptr
    inode_start = (inode_t*)(boot_block + 1);                                  //offset boot block ptr 4kB to start of inodes
    data_block_start = (data_block_t*)(boot_block + boot_block->inodes_count + 1);  //offset boot block ptr to datablocks at the end of inodes
    curr_dentry_idx = 0;                                                            //initialize current directory to 0
}

/* ~~~~~~~~~~~~~~~~~~~~ END OF INITITIALIZATION ~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~ HELPER FUNCTIONS ~~~~~~~~~~~~~~~~~~~~ */

/*
 * read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * Description: Finds file with matching name, copies to dentry
 * Inputs: fname - base address of file
 *         dentry - dentry to be written to
 * Outputs: SUCCESS, FAILURE
 * Side Effects: dentry written
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    /* check if valid fname and dentry */
    if((fname==NULL)||(fname[0]=='\0')||(dentry==NULL)){
        return FAILURE;
    }
    uint32_t fname_length;
    fname_length = strlen((int8_t*)fname) + 1;      //strlen returns 0 indexed length, must add 1
    /* if name is too big, use max fname length */
    if (fname_length == FNAME_LEN + 1) { fname_length = FNAME_LEN; }
    else if(fname_length > FNAME_LEN + 1){
        return -1;
    }
    uint32_t search_length;
    int i; //iterator
    /* check directory entries in boot block */
    for(i=0; i<(boot_block->dentries_count); i++){
        search_length = strlen((const int8_t*)(boot_block->dentries[i].fname)) + 1; //strlen returns 0 indexed len, must add 1
        /* if search name is too big, use max fname length */
        if(search_length > FNAME_LEN){
            search_length = (FNAME_LEN);
        }
        /* check length is the same and name is the same*/
        if((fname_length==search_length)&&(strncmp((const int8_t*)fname, (const int8_t*)(boot_block->dentries[i].fname), fname_length)==0)){
            
            return read_dentry_by_index(i, dentry);    //fill dentry data
        }
    }
    //printf("read_dentry_by_name failed\n");
    return FAILURE; //no file found
}

/*
 * read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * Description: copies statistics to dentry
 * Inputs: index - dentry index
 *         dentry - dentry to be written to
 * Outputs: SUCCESS, FAILURE
 * Side Effects: dentry written
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    /* check if valid dentry and valid index */
    uint32_t n_dentries;
    n_dentries = boot_block->dentries_count;
    if((index<0)||(index>n_dentries)||(dentry==NULL)){
        //printf("read_dentry_by_index fail\n");
        return FAILURE;
    }
    /* copy from index to dentry */
    memcpy(dentry, &(boot_block->dentries[index]), DENTRY_SIZE);
    //printf("read_dentry_by_index good\n");
    return SUCCESS;
}
/*
 * read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Description: Reads data bytes from a file starting from offset and copies
 *              the bytes into a buffer
 * Inputs: inode  - index node of file to be read
 *         offset - positioin in file to start reading from
 *         buf    - location to copy bytes to
 *         length - number of bytes to be read
 * Outputs: number of bytes read or FAILURE
 * Side Effects: data copied to buf
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    //check if valid inode, offset, buf
    uint32_t n_inodes = boot_block->inodes_count;
    if((inode<0)||(inode>n_inodes)||(offset<0)||(buf==NULL)){
        return FAILURE;
    }

    inode_t* inode_local = &inode_start[inode];   //copy of inode
    if(offset>=inode_local->length){
        return 0;                               //At end of file, 0 bytes read
    }
    uint32_t bytes_read = 0;                    //counter of bytes read
    uint32_t nbytes = length;   //number of bytes to read
    //find number of bytes to read
    if((inode_local->length - offset) < length){
        nbytes = inode_local->length - offset;
    }
    uint32_t data_block_idx = offset/BLOCK_SIZE;        //Find block
    uint32_t start = offset % BLOCK_SIZE;               //Find offset w/ respect to block
    uint32_t end = start + nbytes;                      //end of bytes to be read
    uint32_t i = start;
    uint32_t copy_bytes;                                //number of bytes to copy
    //loop until end of file or required length
    while(i<end){
        copy_bytes = BLOCK_SIZE;                        //bytes to copy should be either the size of the block
        if(nbytes - bytes_read < copy_bytes){           //or the number of bytes requested
            copy_bytes = nbytes - bytes_read;
        }
        uint32_t increment = i % BLOCK_SIZE;
        //copy memory
        memcpy(buf, data_block_start[inode_local->data_block[data_block_idx]].data + increment, copy_bytes);
        //increment everything my number of bytes copied
        bytes_read += copy_bytes;
        buf += copy_bytes;
        i += copy_bytes;
        data_block_idx++;
    }
    //previous code
    /*
    while(bytes_read<length){
        // if at end of file break
        if(bytes_read+offset >= inode_local->length){
            break;
        }
        //calculate idx for data_block
        data_block_idx = (bytes_read + offset)/BLOCK_SIZE;
        //calculate buf ptr
        buf_ptr = data_block_start[inode_local->data_block[data_block_idx]].data + ((bytes_read+offset)%BLOCK_SIZE);
        // copy data
        buf[bytes_read] = *buf_ptr;
        bytes_read++;
    }
    */
    return bytes_read;
}

/*
inode_t get_inode(uint32_t inode_idx) {
    return inode_start[inode_idx];
}
*/

/* ~~~~~~~~~~~~~~~~~~~~ END OF HELPER FUNCTIONS ~~~~~~~~~~~~~~~~~~~~ */

/* ~~~~~~~~~~~~~~~~~~~~ FILE OPERATIONS ~~~~~~~~~~~~~~~~~~~~ */

/*
 * file_open(const uint8_t* filename);
 * Description: Opens file of given fname
 * Inputs: filename - name of file
 * Outputs: SUCCESS or FAILURE
 * Side Effects: changes curr_file
 */
int32_t file_open(const uint8_t* filename){
    //dentry_t curr_file;
    //curr_file_location = 0;
    return read_dentry_by_name(filename, &curr_file);
    //return SUCCESS;
}

/*
 * int32_t file_close(uint32_t fd);
 * Description: Closes file. Nothing to be done.
 * Inputs: fd - file descriptor
 * Outputs: SUCCESS or FAILURE
 * Side Effects: none
 */
int32_t file_close(int32_t fd){
    pcb_t* curr_pcb = (pcb_t*)find_pcb();
    curr_pcb->file_array[fd].inode = -1;        //turn process off
    return SUCCESS;
}

/*
 * file_read(uint32_t fd, void* buf, uint32_t length);
 * Description: Copies file into buffer, reads filename by filename
 * Inputs: fd - file descriptor
 *         buf - location to copy bytes
 *         length - number of bytes to read             
 * Outputs: # of bytes read in filename or FAILURE
 * Side Effects: copies data to buf
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
   
    /* check for file */
    pcb_t* curr_pcb = find_pcb();
    int32_t Rbytes;
    if(buf==NULL){
        return FAILURE;
    }
    /* check for file */
    uint32_t curr_inode = (uint32_t)curr_pcb->file_array[fd].inode;                  //get inode number
    uint32_t check_len = inode_start[curr_inode].length;                             //get file len
    uint32_t curr_file_location = (uint32_t)curr_pcb->file_array[fd].file_position;  //get current position
    //check if requested bytes is larger than file
    if(nbytes>check_len){
        nbytes=check_len;
    }
    //copy data to buf
    Rbytes = read_data(curr_inode, curr_file_location, buf, nbytes);
    //update file location
    curr_file_location += Rbytes;
    curr_pcb->file_array[fd].file_position = curr_file_location;
    if(Rbytes==0){
        curr_pcb->file_array[fd].file_position = 0;     //no bytes read, reset position
    }
    return Rbytes;
    /*
    if(read_dentry_by_name((uint8_t*)fd, &dentry)==0){
        //read data to buf
        Rbytes =  read_data(dentry.inode_num, curr_file_location, buf, nbytes);
        curr_file_location += (uint32_t)Rbytes;
	return Rbytes;
    }
    return FAILURE;
    */
}

/*
 * file_write(int32_t fd, const void* buf, int32_t nbytes)
 * Description: Writes into file (does nothing)
 * Inputs: fd - file descriptor
 *         buf - location to write to
 *         length - number of bytes to write             
 * Outputs: FAILURE
 * Side Effects: writes to buf
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return FAILURE;
}

/* ~~~~~~~~~~~~~~~~~~~~ END OF FILE OPERATIONS ~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~ DIRECTORY OPERATIONS ~~~~~~~~~~~~~~~~~~~~ */

/*
 * dir_open(const uint8_t* filename);
 * Description: Opens directory of given fname
 * Inputs: filename - name of directory
 * Outputs: SUCCESS or FAILURE
 * Side Effects: changes curr_file
 */
int32_t directory_open(const uint8_t* filename){
    //dentry_t curr_file;
    curr_dentry_idx = 0;
    //read_dentry_by_name(filename, &curr_file);
    return SUCCESS;
}

/*
 * int32_t dir_close(uint32_t fd);
 * Description: Closes directory. Nothing to be done.
 * Inputs: fd - file descriptor
 * Outputs: SUCCESS or FAILURE
 * Side Effects: none
 */
int32_t directory_close(int32_t fd){
    curr_dentry_idx = 0;
    return SUCCESS;
}

/*
 * dir_read(uint32_t fd, void* buf, uint32_t length);
 * Description: Copies directory into buffer, reads filename by filename
 * Inputs: fd - file descriptor
 *         buf - location to copy bytes
 *         length - number of bytes to read             
 * Outputs: # of bytes read in filename or FAILURE
 * Side Effects: copies data to buf
 */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    dentry_t dentry;
    if(buf==NULL){
        return FAILURE;
    }
    
    //Check if curr_dentry idx is has reached end of dentry
    if(curr_dentry_idx>=boot_block->dentries_count){
        curr_dentry_idx = 0;
        return 0;
    }
    //check if valid idx and copy dentry
    if(read_dentry_by_index(curr_dentry_idx, &dentry)==0){
        uint32_t len = strlen((int8_t*)dentry.fname) + 1;   //strlen returns 0 indexed length. Must add 1
        //check if len is greater than max
        if(len>FNAME_LEN){
            len = FNAME_LEN;
        }
        //copy filename to buffer
        strncpy((int8_t*)buf, (int8_t*)dentry.fname, len);
        curr_dentry_idx++;  // increment dir_read position
        return len;
    } else{
        //reset dentry index if not
        curr_dentry_idx = 0;
        return 0;
    }
    
}

/*
 * directory_write(int32_t fd, const void* buf, int32_t nbytes)
 * Description: Writes into directory (does nothing)
 * Inputs: fd - file descriptor
 *         buf - location to write to
 *         length - number of bytes to write             
 * Outputs: FAILURE
 * Side Effects: writes to buf
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return FAILURE;
}

/* ~~~~~~~~~~~~~~~~~~~~ END OF DIRECTORY OPERATIONS ~~~~~~~~~~~~~~~~~~~~ */
