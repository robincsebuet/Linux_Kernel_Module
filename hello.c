#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/stringify.h>
#include <linux/ktime.h>
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/vfs.h>
#include <linux/dirent.h>
#include <linux/fs_struct.h>
#include <linux/sched.h>
#include <linux/rculist.h>
#include <asm/processor.h>

#define MAX_BUFFER_SIZE 	4*1024*1024
//#define TOTAL_SECONDS_PER_YEAR (365*24*60*60+5*60*60+48*60+46)
#define TOTAL_SECONDS_PER_YEAR (365*24*60*60)
#define TOTAL_SECONDS_PER_DAY (24*60*60)
#define TOTAL_SECONDS_PER_HOUR (60*60)

char * LKM_PROC_COUNT = "proc_count";
char * LKM_PROC_DETAILS = "proc_details";
char * LKM_CPU_INFO = "cpu_info";
char * LKM_CURRENT_TIME = "current_time";
long unsigned int len, temp;
struct timespec system_time;
struct timeval * system_time_in_timeval_Struct;
struct timezone system_time_zone;
char *msg;
struct proc_dir_entry * pointer_to_proc_dir_entry;
struct task_struct *task_list;

static char daytab[2][13] = { { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,
		31 }, { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } };

struct lkm_date {
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;
};
void my_lkm_cpu_info(char * cpu_info_string);
void second_to_date(long second, struct lkm_date * cur_time, char * date_string);
int proc_info(char * process_info_string) ;

int read_proc(struct file *filp, char *buf, size_t count, int *offp) {
	printk(KERN_INFO "intitiating export to userspace\n");
//printk(KERN_INFO "pointer address's are %d  %d", *msg, *msg_for_read);
	printk(KERN_INFO "Buffer content is now %s \n", msg);
	if (count > temp) {
		count = temp;
	}
	temp = temp - count;
	copy_to_user(buf, msg, count);
	if (copy_to_user(buf, msg, count)) {
		printk(KERN_ERR "Failed to copy Data to User \n");
		return -EFAULT;
	}

	if (count == 0)
		temp = len;

//vsprintf()

	printk(KERN_INFO "successfully sent %d byte data to userspace\n",
			(int) count);

	return count;

}

int write_proc(struct file *filp, const char *buf, long unsigned int count,
		int *offp) {
//	printk(KERN_INFO "at write start pointer address's are %d  %d", msg,
//			msg_for_read);
	if (count > MAX_BUFFER_SIZE) {
		len = MAX_BUFFER_SIZE;
	} else {
		len = count;
	}
	printk(KERN_INFO " count is %u len is %u \n", count, len);

	copy_from_user(msg, buf, len);
	printk(KERN_INFO "successfully rcvd  %d byte data from userspace\n",
			(int) count);
	printk(KERN_INFO "rcvd String is %s and userspace data is  %s \n", msg,
			buf);
//	printk(KERN_INFO "pointer address's are %d  %d", msg, msg_for_read);
	len = strlen(msg);
	if(strstr(msg,LKM_CURRENT_TIME)!=NULL){
		ktime_get_real_ts(&system_time);
		struct lkm_date cur_date;
		second_to_date(system_time.tv_sec, &cur_date, msg );
	}else if(strstr(msg,LKM_CPU_INFO)!=NULL){
		my_lkm_cpu_info(msg );
	}else if(strstr(msg,LKM_PROC_DETAILS)!=NULL){
		proc_info(msg );
	}



	printk(KERN_INFO "after collecting proc info  buffer length is %d \n", strlen(msg));
	printk(KERN_INFO "Buffer content is  is %s \n", msg);
	len = strlen(msg);
	temp = len;
	return len;
}



struct file_operations proc_fops = { read: read_proc, write: write_proc,};
void create_new_proc_entry(void) {
	pointer_to_proc_dir_entry = proc_create("hello", 0, NULL, &proc_fops);
	if (pointer_to_proc_dir_entry == NULL) {
		printk(KERN_ERR "Failed to create entry in /proc folder!!!!\n");
		return;
	}

	printk(KERN_INFO "Successfully created  entry in /proc folder.\n");
	msg = vmalloc( MAX_BUFFER_SIZE * sizeof(char));
//msg_for_read = msg;
	if (msg == NULL) {
		printk(KERN_ERR "Failed to initiate kernel space memory!!!!\n");
	}
//printk(KERN_INFO "pointer address's are %d  %d", msg, msg_for_read);

}

int lkm_init(void) {
	printk(KERN_INFO "Initiating my LKM\n");
	create_new_proc_entry();

	printk(KERN_INFO "Successfully Initiated my LKM %lu\n", system_time.tv_sec);

	return 0;
}

void lkm_cleanup(void) {
	printk(KERN_INFO "Cleaning my LKM\n");
	vfree(msg);
	proc_remove(pointer_to_proc_dir_entry);
//remove_proc_entry("hello",NULL);
	printk(KERN_INFO "Cleared my LKM\n");
}
void second_to_date(long second, struct lkm_date * cur_time, char * date_string) {
	cur_time->year = (second / TOTAL_SECONDS_PER_YEAR) + 1970;
	long remaining_second = (second % TOTAL_SECONDS_PER_YEAR);
	int day_of_year = remaining_second / TOTAL_SECONDS_PER_DAY;
	remaining_second = remaining_second % TOTAL_SECONDS_PER_DAY;
	int i, leap;

	leap = cur_time->year % 4 == 0 && cur_time->year % 100 != 0
			|| cur_time->year % 400 == 0;
	for (i = 1; day_of_year > daytab[leap][i]; i++)
		day_of_year -= daytab[leap][i];
	cur_time->month = i;
	cur_time->day = day_of_year;

	cur_time->hour = remaining_second / TOTAL_SECONDS_PER_HOUR;
	remaining_second = remaining_second % TOTAL_SECONDS_PER_HOUR;
	cur_time->min = remaining_second / 60;
	cur_time->sec = remaining_second = remaining_second % 60;
	printk(KERN_INFO "Current time is %d-%d-%d::%d.%d.%d\n", cur_time->year,
			cur_time->month, cur_time->day, cur_time->hour, cur_time->min,
			cur_time->sec);
	sprintf(date_string, "\nCurrent time is %d-%d-%d::%d.%d.%d\n",
			cur_time->year, cur_time->month, cur_time->day, cur_time->hour,
			cur_time->min, cur_time->sec);
	printk(KERN_INFO "after formating result1 is %s \n", date_string);
	printk(KERN_INFO "after formating result2 is %s \n", msg);
}

int proc_info(char * process_info_string) {
	int new_len =0,x=0;
	rcu_read_lock();
	for_each_process(task_list)
	{
		if(new_len>MAX_BUFFER_SIZE-550){

			rcu_read_unlock();
			return new_len;

		}
//		else if(x>500){
//			rcu_read_unlock();
//						return new_len;
//		}
		else{
			new_len += sprintf(process_info_string+new_len, "%s %d \n", task_list->comm,
							task_list->pid);
			printk(KERN_INFO "%s %d \n", task_list->comm,task_list->pid);

		}
		x++;
	}
	printk(KERN_INFO "\ntotal numberof process is %d len is %d \n", x, new_len);
	new_len += sprintf(process_info_string+new_len , "\ntotal number of process is %d \n", x);

	rcu_read_unlock();
	return new_len;
}
void my_lkm_cpu_info(char * cpu_info_string) {
	sprintf(cpu_info_string, "\nnum of active cpu's %d\n", num_online_cpus());

}

MODULE_LICENSE("GPL");
module_init(lkm_init);
module_exit(lkm_cleanup);

