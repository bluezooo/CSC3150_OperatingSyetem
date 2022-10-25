#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/kmod.h>
#include <linux/fs.h>
#include <linux/delay.h>
MODULE_LICENSE("GPL");

static struct task_struct *task;

extern int do_execve(struct filename *filename,
		     const char __user *const __user *__argv,
		     const char __user *const __user *__envp);
extern struct filename *getname_kernel(const char *filename);

//implement exec function
int my_exec(void)
{
	int result;
	const char path[] =
		"/tmp/test";

	// const char path[] = "/home/vagrant/csc3150/Assignment_1_120090246/source/program2/test";
	// const char *const argv[] = {path,NULL, NULL};
	// const char *const envp[] = {"HOME=/", "PATH=/sbin:/usre/sbin:/bin:/usr/bin",NULL};
	result = do_execve(getname_kernel(path), NULL, NULL);
	return 0;
}

struct wait_opts {
	enum pid_type wo_type;
	int wo_flags;
	struct pid *wo_pid;
	struct waitid_info *wo_info;
	int wo_stat;
	struct rusage *wo_rusage;
	wait_queue_entry_t child_wait;
	int notask_error;
};
extern long do_wait(struct wait_opts *wo);

//implement wait function
void my_wait(pid_t pid)
{
	int status = 0;
	int return_value;
	int return_signal;
	struct wait_opts wo;
	struct pid *wo_pid = NULL;
	enum pid_type type;
	type = PIDTYPE_PID;
	wo_pid = find_get_pid(pid);
	wo.wo_type = type;
	wo.wo_pid = wo_pid;
	wo.wo_flags = WEXITED | WUNTRACED;
	wo.wo_info = NULL;
	wo.wo_stat = status;
	wo.wo_rusage = NULL;

	printk("[program2] : child process");
	return_value = do_wait(&wo);
	return_signal = wo.wo_stat;
	if (return_signal == 4991) { //stop
		return_signal = 19;
	} else {
		return_signal &= 0x7f;
	}
	switch (return_signal) {
	case 0:
		printk("[program2] : Normal termination");
		break;
	case 1:
		printk("[program2] : get SIGHUP signal");
		break;
	case 2:
		printk("[program2] : get SIGINT signal");
		break;
	case 3:
		printk("[program2] : get SIGQUIT signal");
		break;
	case 4:
		printk("[program2] : get SIGILL signal");
		break;
	case 5:
		printk("[program2] : get SIGTRAP signal");
		break;
	case 6:
		printk("[program2] : get SIGABRT signal");
		break;
	case 7:
		printk("[program2] : get SIGBUS signal");
		break;
	case 8:
		printk("[program2] : get SIGFPE signal");
		break;
	case 9:
		printk("[program2] : get SIGKILL signal");
		break;
	case 11:
		printk("[program2] : get SIGSEGV signal");
		break;
	case 13:
		printk("[program2] : get SIGPIPE signal");
		break;
	case 14:
		printk("[program2] : get SIGALRM signal");
		break;
	case 15:
		printk("[program2] : get SIGTERM signal");
		break;
	case 19:
		printk("[program2] : get SIGSTOP signal");
		break;
	default:
		printk("[program2] : get SIGTERM signal");
		break;
	}
	printk("[program2] : child process terminated");

	//output child process exit status
	printk("[program2] : The return value is %d\n", return_value);
	printk("[program2] : The return signal is %d\n", return_signal);
	put_pid(wo_pid);
	return;
}

extern pid_t kernel_clone(struct kernel_clone_args *kargs);
// extern pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);
//implement fork function
int my_fork(void *argc)
{
	int i;
	pid_t pid;
	struct kernel_clone_args kargs = {
		.flags = SIGCHLD,
		.stack = (unsigned long)&my_exec,
		.stack_size = 0,
		.parent_tid = NULL,
		.child_tid = NULL,
		.tls = 0,
		.exit_signal = SIGCHLD,
	};
	//set default sigaction for current process
	struct k_sigaction *k_action = &current->sighand->action[0];
	for (i = 0; i < _NSIG; i++) {
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}

	/* fork a process using kernel_clone or kernel_thread */
	/* execute a test program in child process */
	pid = kernel_clone(&kargs);
	printk("[program2] : The child process has pid = %d\n", pid);
	printk("[program2] : This is the parent process, pid = %d\n",
	       (int)current->pid);
	/* wait until child process terminates */
	my_wait(pid);
	// }

	return 0;
}

static int __init program2_init(void)
{
	printk("[program2] : Module_init {Yuhang Wang} {120090246}\n");

	/* create a kernel thread to run my_fork */
	printk("[program2] : Module_init create kthread start\n");

	//create a kthread
	task = kthread_create(&my_fork, NULL, "MyThread");
	printk(KERN_INFO "[program2] : Module_init kthread start\n");

	//wake up new thread if ok
	if (!IS_ERR(task)) {
		wake_up_process(task);
	}

	return 0;
}

static void __exit program2_exit(void)
{
	printk("[program2] : Module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);
