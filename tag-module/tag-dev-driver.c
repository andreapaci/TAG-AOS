#include "tag-dev-driver.h"

static struct cdev cdev;
static struct class *char_dev_class = NULL;
static int major;

static int      dev_open    (struct inode* inode, struct file* filp);
static int      dev_release (struct inode* inode, struct file* filp);
static ssize_t  dev_write   (struct file* filp, const char* buf, size_t size, loff_t *off);
static ssize_t  dev_read    (struct file* filp, char* buf, size_t size, loff_t *off);
static long     dev_ioctl   (struct file* filp, unsigned int command, unsigned long param);
static loff_t   dev_llseek(struct file *filp, loff_t off, int whence);

static char* append_buffer(char* dest, char* source, int* nbytes, int* curr_block);



static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = dev_open,
    .write          = dev_write,
    .read           = dev_read,
    .release        = dev_release,
    .unlocked_ioctl = dev_ioctl,
    .llseek         = dev_llseek,
};

void register_chardev(void) {

    dev_t dev;

    if(unlikely(alloc_chrdev_region(&dev, 0, 0, DEV_NAME) < 0)) {
        printk("%s: Error in allocationg a Char Device region\n", MODNAME);
        return;
    }

    major = MAJOR(dev);

    char_dev_class = class_create(THIS_MODULE, DEV_NAME);
    if(char_dev_class == 0) {
        printk("%s: Error in a Char Device class\n", MODNAME);
        return;
    }

    cdev_init(&cdev, &fops);
    cdev.owner = THIS_MODULE;

    if(unlikely(cdev_add(&cdev, MKDEV(major, 0), 1) < 0)){
        printk("%s: Error in adding char device\n", MODNAME);
        class_destroy(char_dev_class);
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return;
    }

    if(unlikely(device_create(char_dev_class, NULL, MKDEV(major, 0), NULL, DEV_NAME) < 0)) {
        printk("%s: Error in creating device\n", MODNAME);
        cdev_del(&cdev);
        class_destroy(char_dev_class);
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return;
    }
   

    PRINT
    printk("%s: Char device driver registered\n", MODNAME);

    return;
}

void unregister_chardev(void) {

    cdev_del(&cdev);
    device_destroy(char_dev_class, MKDEV(major, 0));
    class_destroy(char_dev_class);

    unregister_chrdev_region(MKDEV(major, 0), 1);

    PRINT
    printk("%s: Char device driver unregistered\n", MODNAME);

}


// Dev open and Dev release are no-op
static int dev_open(struct inode* inode, struct file* filp) {
    return 0;

}
static int dev_release(struct inode* inode, struct file* filp) {
    return 0;
}

static ssize_t dev_read(struct file* filp, char* buf, size_t size, loff_t *off) {

    // Retreive Tag info
    
    int i, j;
    int tag_key, euid;
    int nbytes, curr_block;
    loff_t relative_off;
    char __attribute__((aligned(PAGE_SIZE))) *buffer;
    
    // Since header and divider are always used, there's no point in allocating and filling them every read
    // Instead, it's better to create them on "open" or even better, on "register_chardev"
    char* header;
    char* divider;

    curr_block = 1;
    nbytes = 0;
    
    // This one cannot be done because the size of the "file" is not constant, in fact only
    // created tags will be displayed. If the size was fixed, this initialization of i
    // would seriosly decrease the time to make the read
    //i = (*off) / TAG_SIZE;
    i = 0;
    // Input check
    if(size <= 0) return 0;
    if(buf == 0 || (*off) < 0)  return -1;  
    
    relative_off = *off;

    // Allocate buffer used to store the output
    // + 1 is \0
    buffer = vzalloc(sizeof(char) * (CHAR_BLOCK * curr_block + 1));
    if(unlikely(buffer == 0)) {
        PRINT
        printk("%s: Could not allocate Buffer for dev_read.\n", MODNAME);
        return -1;
    }

    // Allocate and create an header
    // Possibly a single line will not go past 64 chars    
    header = kzalloc(sizeof(char) * 64, GFP_KERNEL);
    if(header == 0) {
        PRINT
        printk("%s: Could not allocate a temporal buffer\n", MODNAME);
        vfree(buffer);
        return -1;
    }
    sprintf(header, "| %10s | %10s | %10s | %10s |\n", "KEY", "EUID", "LEVEL", "WAIT");
    int header_size;
    header_size = strlen(header);

    // Append the header at the begginning
    buffer = strncat(buffer, header, header_size);
    
    // Free the header (no more necessary)
    kfree(header);

    // Allocate and create an divider
    // Possibly a single line will not go past 64 chars    
    divider = kzalloc(sizeof(char) * 64, GFP_KERNEL);
    if(divider == 0) {
        PRINT
        printk("%s: Could not allocate a temporal buffer\n", MODNAME);
        vfree(buffer);
        return -1;
    }

    memset(divider, '-', LINE_SIZE);
    divider[0] = '+';
    divider[LINE_SIZE - 1] = '+';
    divider[LINE_SIZE] = '\n';

    buffer = strcat(buffer, divider);

    




    // For each tag...
    for(; i < MAX_TAGS; i++) {
        
        // Take the lock (so it's not possible to delete it while reading from it)
        if(unlikely(down_read_interruptible(&(tag_lock[i])) == -EINTR)) {                
            PRINT
            printk("%s: RW Lock was interrupted.\n", MODNAME);
            continue;
        }

        tag_t* tag_entry;
        tag_entry = tags[i];

        // Non existing tag (in case the tag is not existing, go to the next iteration)
        if(tag_entry == 0) {
            up_read(&(tag_lock[i]));
            continue;
        }

        // If the offset is greater than the Tag size, there's no point in reading this
        if(relative_off > TAG_SIZE) {
            relative_off -= TAG_SIZE;
            up_read(&(tag_lock[i]));
            continue;
        }

        // Store the KEY and the EUID
        tag_key = tag_entry -> key;
        euid = tag_entry -> euid;

        // For each level
        for(j = 0; j < LEVELS; j++) {
            tag_level_t* tag_level;

            
            // Lock the level-accessing structure
            if(unlikely(down_read_interruptible(&(tag_entry -> level_lock[j])) == -EINTR)) {                
                PRINT
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                continue;
            }

            tag_level = (tag_entry -> tag_level)[j];

            if(tag_level == 0) {
                up_read(&(tag_entry -> level_lock[j]));
                continue;
            }

            if(unlikely(down_read_interruptible(&(tag_level -> rcu_lock)) == -EINTR)) {                
                PRINT
                printk("%s: RW Lock was interrupted.\n", MODNAME);
                up_read(&(tag_entry -> level_lock[j]));
                continue;
                
            }

            // RCU instance of the "j" level accessed

            up_read(&(tag_entry -> level_lock[j]));

            // Allocate a temporary buffer to store single level info
            // Possibly a single line will not go past 64 chars
            char* temp;
            temp = kzalloc(sizeof(char) * 64, GFP_KERNEL);
            if(temp == 0) {
                PRINT
                printk("%s: Could not allocate a temporal buffer\n", MODNAME);
                up_read(&(tag_level -> rcu_lock));
                continue;
            }

            // Fill the temporary buffer
            sprintf(temp, "| %10d | %10d | %10d | %10d |\n", tag_key, euid, j, atomic_read(&(tag_level -> waiting)));
            
            buffer = append_buffer(buffer, temp, &nbytes, &curr_block);
            if(unlikely(buffer == 0)) {
                PRINT
                printk("%s: Could not allocate Buffer for dev_read.\n", MODNAME);
                up_read(&(tag_level -> rcu_lock));
                up_read(&(tag_lock[i]));
                kfree(temp);
                kfree(divider);
                return -1;
            }

            kfree(temp);

            up_read(&(tag_level -> rcu_lock));

            // If reached the maximum bytes to be read, stop and send the message
            // to the user buffer
            if(nbytes > size + relative_off) { //*off) {
                up_read(&(tag_lock[i]));
                goto exit;
            }

        }

        // Append a divider between levels
        buffer = append_buffer(buffer, divider, &nbytes, &curr_block);
        if(unlikely(buffer == 0)) {
            PRINT
            printk("%s: Could not allocate Buffer for dev_read.\n", MODNAME);
            up_read(&(tag_lock[i]));
            kfree(divider);
            return -1;
        }


        up_read(&(tag_lock[i]));


    }
    
    exit: nop();

    // Compute actual buffer interval to be delivered

    int tot_size;
    tot_size = strlen(buffer);

    
    
    // In case the offset is greater than the total size, must return 0
    if(tot_size <= relative_off) { //(*off)) {
        kfree(divider);
        vfree(buffer);
        return 0;
    }


    if(size > tot_size - relative_off) //(*off))
        size = tot_size - relative_off; //(*off);    


    char* cpy_buffer;
    cpy_buffer = buffer + relative_off; //(*off);

    if(unlikely(copy_to_user(buf, cpy_buffer, size) != 0)) {
        PRINT
        printk("%s: Error in copy_to_user()\n", MODNAME);
        kfree(divider);
        vfree(buffer);
        return -1;
    }

    *off += size;

    kfree(divider);
    vfree(buffer);
    return size;
}




static loff_t dev_llseek(struct file *filp, loff_t off, int whence)
{
  
  loff_t newpos;

  switch(whence) {
   case 0: /* SEEK_SET */ 
    // This operation is needed to avoid re-opening the file to start reading from the beggining
    newpos = off;
    break;

   case 1: /* SEEK_CUR */
    // The CUR should be made here instead of increasing the offset at every dev_read() 
    break;

   case 2: /* SEEK_END */
    // This operation should not be called
    
    return off;
    break;

   default: /* can't happen */
    return -EINVAL;
  }
  if (newpos < 0) return -EINVAL;
  filp->f_pos = newpos;
  return newpos;
}



static ssize_t dev_write(struct file* filp, const char* buf, size_t size, loff_t *off) {
    PRINT
    printk("%s: Write not permitted (Read-Only)\n", MODNAME);
    return -1;
}

static long dev_ioctl(struct file* filp, unsigned int command, unsigned long param) {
    PRINT
    printk("%s: CTL not permitted\n", MODNAME);
    return -1;
}



// Append the source to dest and free the source
// If dest is not big enough, it will allocate a new buffer for dest and return it
// It returns 0 in case the buffer could not be allocated
static char* append_buffer(char* dest, char* source, int* nbytes, int* curr_block) {
    
    int temp_size;
    temp_size = strlen(source);
    
    (*nbytes) += temp_size;

    // If the resulting size is greater than the allocated one, allocate a new buffer
    if(*nbytes > CHAR_BLOCK * (*curr_block)) {
        // Inrease the current needed CHAR_BLOCK
        (*curr_block)++;
        
        // Allocate a new Temp buffer
        char __attribute__((aligned(PAGE_SIZE))) *temp_buff;
        temp_buff = vzalloc(sizeof(char) * (CHAR_BLOCK * (*curr_block) + 1));
        if(unlikely(temp_buff == 0)) {
            vfree(dest);
            return 0;
        }

        // Coopy the content to the new Temporary buffer and free the other
        strcpy(temp_buff, dest);
        vfree(dest);

        // Make the old buffer point to the new, allocated level
        dest = temp_buff;
    }

    // Append the new level information
    dest = strncat(dest, source, temp_size);

    

    return dest;

}

