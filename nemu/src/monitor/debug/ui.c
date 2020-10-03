#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>





void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}


static int cmd_help(char *args);


static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	int step=0;
	if (args == NULL)
      step = 1;
	else 
    sscanf (args,"%d",&step);
  int j;
  for(j=0;j<step;j++)
	  cpu_exec(1);
	return 0;
}


static int cmd_info(char *args) {
  if(args){
    if(args[0]=='r'){
      printf("eax\t\t0x%08x\t\t%010d\n",cpu.eax,cpu.eax);
      printf("ecx\t\t0x%08x\t\t%010d\n",cpu.ecx,cpu.ecx);
      printf("edx\t\t0x%08x\t\t%010d\n",cpu.edx,cpu.edx);
      printf("ebx\t\t0x%08x\t\t%010d\n",cpu.ebx,cpu.ebx);
      printf("esp\t\t0x%08x\t\t0x%010d\n",cpu.esp,cpu.esp);
      printf("ebp\t\t0x%08x\t\t0x%010d\n",cpu.ebp,cpu.ebp);
      printf("esi\t\t0x%08x\t\t%010d\n",cpu.esi,cpu.esi);
      printf("edi\t\t0x%08x\t\t%010d\n",cpu.edi,cpu.edi);
      printf("eip\t\t0x%08x\t\t0x%08x\n",cpu.eip,cpu.eip);
    }
    else if(args[0]=='w'){
      info_wp();
    }
    else{
      cmd_help(args);
    }
  }
  else 
    cmd_help(args);
  return 0;
}


static int cmd_p(char *args) {
	uint32_t step ;
	bool suc;
	step = expr (args,&suc);
	if (suc)
		printf ("0x%x:\t%d\n",step,step);
	else 
    assert (0);
	return 0;
}


static int cmd_x(char *args) {
	uint32_t addr;
  uint32_t n;
  sscanf(args,"%d%x",&n,&addr);
  int i;
  for(i=0;i<n;i++){
    printf("0x%08x\t",swaddr_read(addr+i*4,4));
    if(!((i+1)%8))printf("\n");
  }
  printf("\n");
  return 0;
}



static int cmd_w(char *args) {
	WP *f;
	bool suc;
	f = new_wp();
	printf ("Watchpoint %d: %s\n",f->NO,args);
	f->val = expr (args,&suc);
	strcpy (f->expr,args);
	if (!suc)Assert (1,"wrong\n");
	printf ("Value = %d\n",f->val);
	return 0;
}


static int cmd_d(char *args) {
	int step;
	sscanf (args,"%d",&step);
	delete_wp (step);
	return 0;
}


static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Let the program execute n steps", cmd_si },
  { "info", "Display the register status and the watchpoint information", cmd_info},
  { "x", "Caculate the value of expression and display the content of the address", cmd_x},
  { "p","Calculate an expression", cmd_p},
  { "w", "Create a watchpoint", cmd_w},
  { "d", "Delete a watchpoint", cmd_d},

	/* TODO: Add more commands */
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}



void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}



